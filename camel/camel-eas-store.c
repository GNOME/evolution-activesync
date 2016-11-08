/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Authors: David Woodhouse <dwmw2@infradead.org>
 *
 * Copyright © 2010-2011 Intel Corporation (www.intel.com)
 * 
 * Based on GroupWise/EWS code which was
 *  Copyright © 1999-2008 Novell, Inc. (www.novell.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <libedataserver/libedataserver.h>

#include <eas-folder.h>
#include <libeasmail.h>
#include "camel-eas-folder.h"
#include "camel-eas-store.h"
#include "camel-eas-summary.h"
#include "camel-eas-utils.h"

#ifdef G_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "camel-eas-settings.h"

#define d(x) x
#define CURSOR_ITEM_LIMIT 100

#define CAMEL_EAS_STORE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_EAS_STORE, CamelEasStorePrivate))

#define FINFO_REFRESH_INTERVAL 60

struct _CamelEasStorePrivate {

	time_t last_refresh_time;
	GMutex get_finfo_lock;
	EasEmailHandler *handler;
};

extern CamelServiceAuthType camel_eas_password_authtype; /*for the query_auth_types function*/

static gboolean
eas_store_construct	(CamelService *service, CamelSession *session,
			 CamelProvider *provider,
			 GError **error);

static void camel_eas_store_initable_init (GInitableIface *interface);
static GInitableIface *parent_initable_interface;

G_DEFINE_TYPE_WITH_CODE (CamelEasStore, camel_eas_store,
			 CAMEL_TYPE_OFFLINE_STORE,
			 G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
						camel_eas_store_initable_init))

static gboolean
eas_store_initable_init		(GInitable *initable,
				 GCancellable *cancellable,
				 GError **error)
{
	CamelService *service = CAMEL_SERVICE (initable);
	CamelSession *session = camel_service_ref_session (service);
	gboolean ret;

	/* Chain up to parent interface's init() method. */
	if (!parent_initable_interface->init (initable, cancellable, error))
		return FALSE;

	ret = eas_store_construct (service, session, NULL,
				   error);

	g_object_unref(session);
	/* Add transport here ? */

	return ret;
}

static void
camel_eas_store_initable_init (GInitableIface *interface)
{
	parent_initable_interface = g_type_interface_peek_parent (interface);

	interface->init = eas_store_initable_init;
}

static gboolean
eas_store_construct	(CamelService *service, CamelSession *session,
			 CamelProvider *provider,
			 GError **error)
{
	CamelEasStore *eas_store;
	gchar *summary_file, *session_storage_path;

	eas_store = (CamelEasStore *) service;

	/* Disable virtual trash and junk folders. Exchange has real
	   folders for that */
	camel_store_set_flags (CAMEL_STORE (eas_store), camel_store_get_flags (CAMEL_STORE (eas_store)) & ~(CAMEL_STORE_VTRASH | CAMEL_STORE_VJUNK));

	/*storage path*/
	session_storage_path = g_strdup (camel_service_get_user_data_dir (service));
	if (!session_storage_path) {
		g_set_error (
			error, CAMEL_STORE_ERROR,
			CAMEL_STORE_ERROR_INVALID,
			_("Session has no storage path"));
		return FALSE;
	}
	eas_store->storage_path = session_storage_path;

	g_mkdir_with_parents (eas_store->storage_path, 0700);
	summary_file = g_build_filename (eas_store->storage_path, "folder-tree-v2", NULL);
	eas_store->summary = camel_eas_store_summary_new (summary_file);
	camel_eas_store_summary_load (eas_store->summary, NULL);

	g_free (summary_file);
	return TRUE;
}

static gboolean
eas_connect_sync (CamelService *service, GCancellable *cancellable, GError **error)
{
	CamelEasSettings *settings;
	CamelEasStore *eas_store;
	CamelEasStorePrivate *priv;
	const gchar *account_uid;

	eas_store = (CamelEasStore *) service;
	priv = eas_store->priv;

	if (camel_service_get_connection_status (service) == CAMEL_SERVICE_DISCONNECTED)
		return FALSE;

	if (priv->handler)
		return TRUE;

	settings = CAMEL_EAS_SETTINGS (camel_service_ref_settings (service));

	account_uid = camel_eas_settings_get_account_uid (settings);
	g_object_unref(settings);

	if (!account_uid) {
		g_set_error (
			error, CAMEL_STORE_ERROR,
			CAMEL_STORE_ERROR_INVALID,
			_("EAS service has no account UID"));
		return FALSE;
	}

	priv->handler = eas_mail_handler_new (account_uid, error);
	if (!priv->handler) {
		camel_service_disconnect_sync (service, TRUE, cancellable, NULL);
		return FALSE;
	}

	camel_offline_store_set_online_sync (
		CAMEL_OFFLINE_STORE (eas_store), TRUE, cancellable, NULL);

	return TRUE;
}

static gboolean
eas_disconnect_sync (CamelService *service, gboolean clean, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store = (CamelEasStore *) service;
	CamelServiceClass *service_class;

	service_class = CAMEL_SERVICE_CLASS (camel_eas_store_parent_class);
	if (!service_class->disconnect_sync (service, clean, cancellable, error))
		return FALSE;

	/* TODO cancel all operations in the connection */
	g_object_unref (eas_store->priv->handler);
	eas_store->priv->handler = NULL;

	return TRUE;
}

static  GList*
eas_store_query_auth_types_sync (CamelService *service, GCancellable *cancellable, GError **error)
{
	GList *auth_types = NULL;

	d(printf("in query auth types\n"));
	auth_types = g_list_prepend (auth_types,  &camel_eas_password_authtype);
	return auth_types;
}

static CamelFolderInfo* eas_create_folder_sync (CamelStore *store, const gchar *parent_name,const gchar *folder_name,GCancellable *cancellable,GError **error);

static CamelFolder *
eas_get_folder_sync (CamelStore *store, const gchar *folder_name, guint32 flags, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store;
	CamelFolder *folder = NULL;
	gchar *fid, *folder_dir;

	eas_store = (CamelEasStore *) store;

	fid = camel_eas_store_summary_get_folder_id_from_name (eas_store->summary, folder_name);

	/* We don't support CAMEL_STORE_FOLDER_EXCL. Nobody ever uses it */
	if (!fid && (flags & CAMEL_STORE_FOLDER_CREATE)) {
		CamelFolderInfo *fi;
		const gchar *parent, *top, *slash;
		gchar *copy = NULL;

		slash = strrchr (folder_name, '/');
		if (slash) {
			copy = g_strdup (folder_name);

			/* Split into parent path, and new name */
			copy[slash - folder_name] = 0;
			parent = copy;
			top = copy + (slash - folder_name) + 1;
		} else {
			parent = "";
			top = folder_name;
		}

		fi = eas_create_folder_sync (store, parent, top, cancellable, error);
		g_free (copy);

		if (!fi)
			return NULL;

		camel_folder_info_free (fi);
		fid = camel_eas_store_summary_get_folder_id_from_name (eas_store->summary, folder_name);
	}

	if (!fid) {
		g_set_error (error, CAMEL_STORE_ERROR,
			     CAMEL_ERROR_GENERIC,
			     _("No such folder: %s"), folder_name);
		return NULL;
	}

	folder_dir = g_build_filename (eas_store->storage_path, "folders", folder_name, NULL);
	folder = camel_eas_folder_new (store, folder_name, folder_dir, fid, cancellable, error);

	g_free (folder_dir);
	g_free (fid);

	return folder;
}

static CamelFolderInfo *
folder_info_from_store_summary (CamelEasStore *store, const gchar *top, guint32 flags, GError **error)
{
	CamelEasStoreSummary *eas_summary;
	GSList *folders, *l;
	GPtrArray *folder_infos;
	CamelFolderInfo *root_fi = NULL;
	CamelFolderSummary *s;

	eas_summary = store->summary;
	folders = camel_eas_store_summary_get_folders (eas_summary, top);

	if (!folders)
		return NULL;

	folder_infos = g_ptr_array_new ();

	s = g_object_new (CAMEL_TYPE_EAS_SUMMARY, NULL);
	for (l = folders; l != NULL; l = g_slist_next (l)) {
		CamelFolderInfo *fi;
		gint64 ftype;

		ftype = camel_eas_store_summary_get_folder_type (eas_summary, l->data, NULL);
		if (!eas_folder_type_is_mail (ftype) &&
		    ftype != EAS_FOLDER_TYPE_USER_CREATED_GENERIC)
			continue;

		fi = camel_eas_utils_build_folder_info (store, l->data);
		if (!camel_folder_summary_header_load (s, CAMEL_STORE (store), fi->full_name, NULL)) {
			fi->unread = camel_folder_summary_get_unread_count (s);
			fi->total = camel_folder_summary_get_saved_count (s);
		}
		g_ptr_array_add	(folder_infos, fi);
	}

	root_fi = camel_folder_info_build (folder_infos, top, '/', TRUE);

	g_ptr_array_free (folder_infos, TRUE);
	g_slist_foreach (folders, (GFunc) g_free, NULL);
	g_slist_free (folders);
	g_object_unref (s);

	return root_fi;
}

static CamelFolderInfo *
eas_get_folder_info_sync (CamelStore *store, const gchar *top, guint32 flags, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store;
	CamelEasStorePrivate *priv;
	CamelFolderInfo *fi = NULL;
	GSList *new_folders = NULL;

	eas_store = (CamelEasStore *) store;
	priv = eas_store->priv;

	g_mutex_lock (&priv->get_finfo_lock);
	if (!(camel_offline_store_get_online (CAMEL_OFFLINE_STORE (store))
	      && camel_service_connect_sync ((CamelService *)store, cancellable, error))) {
		g_mutex_unlock (&priv->get_finfo_lock);
		goto offline;
	}

	if (!eas_mail_handler_get_folder_list (eas_store->priv->handler,
					       FALSE,
					       &new_folders, cancellable, error)) {
		if (error)
			g_warning ("Unable to fetch the folder hierarchy: %s :%d \n",
				   (*error)->message, (*error)->code);
		else
			g_warning ("Unable to fetch the folder hierarchy.\n");

		g_mutex_unlock (&priv->get_finfo_lock);
		return NULL;
	}
	eas_utils_sync_folders (eas_store, new_folders);
	g_mutex_unlock (&priv->get_finfo_lock);

offline:
	fi = folder_info_from_store_summary ( (CamelEasStore *) store, top, flags, error);
	return fi;
}

static CamelFolderInfo*
eas_create_folder_sync (CamelStore *store,
		const gchar *parent_name,
		const gchar *folder_name,
		GCancellable *cancellable,
		GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Create folder not possible with ActiveSync"));
	return NULL;
}

static gboolean
eas_delete_folder_sync	(CamelStore *store,
			 const gchar *folder_name,
			 GCancellable *cancellable,
			 GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Delete folder not possible with ActiveSync"));
	return FALSE;
}


static gboolean
eas_rename_folder_sync	(CamelStore *store,
			const gchar *old_name,
			const gchar *new_name,
			GCancellable *cancellable,
			GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Rename folder not possible with ActiveSync"));
	return FALSE;
}

gchar *
eas_get_name (CamelService *service, gboolean brief)
{
	CamelStoreSettings *settings = CAMEL_STORE_SETTINGS (camel_service_ref_settings (service));
	const char *account_uid = camel_eas_settings_get_account_uid ((CamelEasSettings *) settings);
	/* Account UID is nothing but the email or user@host */
	char **strings;
	char *ret;

	g_object_unref(settings);
	strings = g_strsplit (account_uid, "@", 0);
	if (brief)
		ret = g_strdup_printf(_("ActiveSync server %s"),
				       strings[1]);
	else
		ret = g_strdup_printf(_("ActiveSync service for %s on %s"),
				       strings[0], strings[1]);
	
	g_strfreev (strings);

	return ret;
}

EasEmailHandler *
camel_eas_store_get_handler (CamelEasStore *eas_store)
{
	return g_object_ref (eas_store->priv->handler);
}

static CamelFolder *
eas_get_trash_folder_sync (CamelStore *store, GCancellable *cancellable, GError **error)
{
	return NULL;
}

static gboolean
eas_can_refresh_folder (CamelStore *store, CamelFolderInfo *info, GError **error)
{
	CamelStoreSettings *settings = CAMEL_STORE_SETTINGS (camel_service_ref_settings (CAMEL_SERVICE(store)));
	gboolean ret;

	/* Skip unselectable folders from automatic refresh */
	if (info && (info->flags & CAMEL_FOLDER_NOSELECT) != 0) return FALSE;

	ret = CAMEL_STORE_CLASS(camel_eas_store_parent_class)->can_refresh_folder (store, info, error) || camel_eas_settings_get_check_all ((CamelEasSettings *)settings);
	g_object_unref(settings);
	return ret;
}

gboolean
camel_eas_store_connected (CamelEasStore *eas_store, GCancellable *cancellable, GError **error)
{

	if (!camel_offline_store_get_online (CAMEL_OFFLINE_STORE (eas_store))) {
		g_set_error (
			error, CAMEL_SERVICE_ERROR,
			CAMEL_SERVICE_ERROR_UNAVAILABLE,
			_("You must be working online to complete this operation"));
		return FALSE;
	}

	if (!camel_service_connect_sync ((CamelService *) eas_store, cancellable, error))
		return FALSE;

	return TRUE;
}

static void
eas_store_dispose (GObject *object)
{
	CamelEasStore *eas_store;

	eas_store = CAMEL_EAS_STORE (object);

	if (eas_store->summary != NULL) {
		camel_eas_store_summary_save (eas_store->summary, NULL);
		g_object_unref (eas_store->summary);
		eas_store->summary = NULL;
	}

	if (eas_store->priv->handler != NULL) {
		g_object_unref (eas_store->priv->handler);
		eas_store->priv->handler = NULL;
	}

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (camel_eas_store_parent_class)->dispose (object);
}

static void
eas_store_finalize (GObject *object)
{
	CamelEasStore *eas_store;

	eas_store = CAMEL_EAS_STORE (object);

	g_free (eas_store->storage_path);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (camel_eas_store_parent_class)->finalize (object);
}

static void
camel_eas_store_class_init (CamelEasStoreClass *class)
{
	GObjectClass *object_class;
	CamelServiceClass *service_class;
	CamelStoreClass *store_class;

	g_type_class_add_private (class, sizeof (CamelEasStorePrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = eas_store_dispose;
	object_class->finalize = eas_store_finalize;

	service_class = CAMEL_SERVICE_CLASS (class);
	service_class->settings_type = CAMEL_TYPE_EAS_SETTINGS;	

	service_class->query_auth_types_sync = eas_store_query_auth_types_sync;
	service_class->get_name = eas_get_name;
	service_class->connect_sync = eas_connect_sync;
	service_class->disconnect_sync = eas_disconnect_sync;

	store_class = CAMEL_STORE_CLASS (class);
	store_class->get_folder_sync = eas_get_folder_sync;
	store_class->create_folder_sync = eas_create_folder_sync;
	store_class->delete_folder_sync = eas_delete_folder_sync;
	store_class->rename_folder_sync = eas_rename_folder_sync;
	store_class->get_folder_info_sync = eas_get_folder_info_sync;

	store_class->get_trash_folder_sync = eas_get_trash_folder_sync;
	store_class->can_refresh_folder = eas_can_refresh_folder;
}

static void
camel_eas_store_init (CamelEasStore *eas_store)
{
	eas_store->priv =
		CAMEL_EAS_STORE_GET_PRIVATE (eas_store);

	eas_store->priv->handler = NULL;
	eas_store->priv->last_refresh_time = time (NULL) - (FINFO_REFRESH_INTERVAL + 10);
	g_mutex_init(&eas_store->priv->get_finfo_lock);
}
