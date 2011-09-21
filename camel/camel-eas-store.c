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

#include <libedataserver/e-flag.h>

#include <eas-folder.h>
#include <libeasmail.h>
#include "camel-eas-compat.h"
#include "camel-eas-folder.h"
#include "camel-eas-store.h"
#include "camel-eas-summary.h"
#include "camel-eas-utils.h"

#ifdef G_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define d(x) x
#define CURSOR_ITEM_LIMIT 100

#define CAMEL_EAS_STORE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_EAS_STORE, CamelEasStorePrivate))

#define FINFO_REFRESH_INTERVAL 60

struct _CamelEasStorePrivate {

	gchar *account_uid;
	time_t last_refresh_time;
	GMutex *get_finfo_lock;
	EasEmailHandler *handler;
};

#if ! EDS_CHECK_VERSION(2,33,0)
static inline void camel_offline_store_set_online_sync(CamelOfflineStore *store,
						       gboolean online,
						       GCancellable *cancellable,
						       GError *error)
{
	camel_offline_store_set_network_state(store,
			online ? CAMEL_OFFLINE_STORE_NETWORK_AVAIL :
				 CAMEL_OFFLINE_STORE_NETWORK_UNAVAIL,
			NULL);
}

static inline gboolean camel_offline_store_get_online(CamelOfflineStore *store)
{
	return (camel_offline_store_get_network_state(store, NULL) ==
		CAMEL_OFFLINE_STORE_NETWORK_AVAIL);
}
#endif

extern CamelServiceAuthType camel_eas_password_authtype; /*for the query_auth_types function*/

static gboolean
eas_store_construct	(CamelService *service, CamelSession *session,
			 CamelProvider *provider, CamelURL *url,
			 GError **error);

#if EDS_CHECK_VERSION(3,1,0)
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
	CamelService *service;
	CamelSession *session;
	CamelURL *url;
	gboolean ret;

	service = CAMEL_SERVICE (initable);
	url = camel_service_get_camel_url (service);
	session = camel_service_get_session (service);

	/* Chain up to parent interface's init() method. */
	if (!parent_initable_interface->init (initable, cancellable, error))
		return FALSE;

	ret = eas_store_construct (service, session, NULL, url, error);

	/* Add transport here ? */

	return ret;
}

static void
camel_eas_store_initable_init (GInitableIface *interface)
{
	parent_initable_interface = g_type_interface_peek_parent (interface);

	interface->init = eas_store_initable_init;
}

#else
G_DEFINE_TYPE (CamelEasStore, camel_eas_store, CAMEL_TYPE_OFFLINE_STORE)
#endif

static gboolean
eas_store_construct	(CamelService *service, CamelSession *session,
			 CamelProvider *provider, CamelURL *url,
			 GError **error)
{
	CamelEasStore *eas_store;
	CamelEasStorePrivate *priv;
	gchar *summary_file, *session_storage_path;
#if ! EDS_CHECK_VERSION(3,1,0)
	CamelServiceClass *service_class;

	/* Chain up to parent's construct() method. */
	service_class = CAMEL_SERVICE_CLASS (camel_eas_store_parent_class);
	if (!service_class->construct (service, session, provider, url, error))
		return FALSE;
#endif

	eas_store = (CamelEasStore *) service;
	priv = eas_store->priv;

	/* Disable virtual trash and junk folders. Exchange has real
	   folders for that */
	((CamelStore *)eas_store)->flags &= ~(CAMEL_STORE_VTRASH|CAMEL_STORE_VJUNK);

	/*storage path*/
	session_storage_path = camel_session_get_storage_path (session, service, error);
	if (!session_storage_path) {
		g_set_error (
			error, CAMEL_STORE_ERROR,
			CAMEL_STORE_ERROR_INVALID,
			_("Session has no storage path"));
		return FALSE;
	}
	eas_store->storage_path = session_storage_path;

	priv->account_uid = g_strdup (camel_url_get_param (url, "account_uid"));
	if (!priv->account_uid) {
		g_set_error (
			error, CAMEL_STORE_ERROR,
			CAMEL_STORE_ERROR_INVALID,
			_("EAS service has no account UID"));
		return FALSE;
	}

	g_mkdir_with_parents (eas_store->storage_path, 0700);
	summary_file = g_build_filename (eas_store->storage_path, "folder-tree-v2", NULL);
	eas_store->summary = camel_eas_store_summary_new (summary_file);
	camel_eas_store_summary_load (eas_store->summary, NULL);

	g_free (summary_file);
	return TRUE;
}

static guint
eas_hash_folder_name (gconstpointer key)
{
	return g_str_hash (key);
}

static gint
eas_compare_folder_name (gconstpointer a, gconstpointer b)
{
	gconstpointer aname = a, bname = b;

	return g_str_equal (aname, bname);
}

static gboolean
eas_connect_sync (CamelService *service, EVO3(GCancellable *cancellable,) GError **error)
{
	EVO2(GCancellable *cancellable = NULL;)
	CamelEasStore *eas_store;
	CamelEasStorePrivate *priv;

	eas_store = (CamelEasStore *) service;
	priv = eas_store->priv;

	if (camel_service_get_connection_status (service) == CAMEL_SERVICE_DISCONNECTED)
		return FALSE;

	camel_service_lock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);

	if (priv->handler) {
		camel_service_unlock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);
		return TRUE;
	}

	priv->handler = eas_mail_handler_new (priv->account_uid, error);
	if (!priv->handler) {
		camel_service_unlock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);
		EVO3_sync(camel_service_disconnect) (service, TRUE, NULL);
		return FALSE;
	}

	camel_offline_store_set_online_sync (
		CAMEL_OFFLINE_STORE (eas_store), TRUE, cancellable, NULL);

	camel_service_unlock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);

	return TRUE;
}

static gboolean
eas_disconnect_sync (CamelService *service, gboolean clean, EVO3(GCancellable *cancellable,) GError **error)
{
	CamelEasStore *eas_store = (CamelEasStore *) service;
	CamelServiceClass *service_class;

	service_class = CAMEL_SERVICE_CLASS (camel_eas_store_parent_class);
	if (!service_class->EVO3_sync(disconnect) (service, clean, EVO3(cancellable,) error))
		return FALSE;

	camel_service_lock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);

	/* TODO cancel all operations in the connection */
	g_object_unref (eas_store->priv->handler);
	eas_store->priv->handler = NULL;

	camel_service_unlock (service, CAMEL_SERVICE_REC_CONNECT_LOCK);

	return TRUE;
}

static  GList*
eas_store_query_auth_types_sync (CamelService *service, EVO3(GCancellable *cancellable,) GError **error)
{
	GList *auth_types = NULL;

	d(printf("in query auth types\n"));
	auth_types = g_list_prepend (auth_types,  &camel_eas_password_authtype);
	return auth_types;
}

static CamelFolderInfo* eas_create_folder_sync (CamelStore *store, const gchar *parent_name,const gchar *folder_name,EVO3(GCancellable *cancellable,)GError **error);

static CamelFolder *
eas_get_folder_sync (CamelStore *store, const gchar *folder_name, guint32 flags, EVO3(GCancellable *cancellable,) GError **error)
{
	EVO2(GCancellable *cancellable = NULL;)
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

		fi = eas_create_folder_sync (store, parent, top, EVO3(cancellable,) error);
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
		if (!camel_folder_summary_header_load_from_db (s, CAMEL_STORE (store),
							       fi->full_name, NULL)) {
			fi->unread = s->unread_count;
			fi->total = s->saved_count;
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

#if !EDS_CHECK_VERSION(3,1,0)
/* in Evo 3.1 and onwards, the storage path is based on UID anyway, so
   we don't need this */
static gchar *
eas_service_get_path (CamelService *service)
{
	return g_build_filename (service->provider->protocol,
				 camel_url_get_param (camel_service_get_camel_url(service),
						      "account_uid"),
				 NULL);
}
#endif

static CamelFolderInfo *
eas_get_folder_info_sync (CamelStore *store, const gchar *top, guint32 flags, EVO3(GCancellable *cancellable,) GError **error)
{
	EVO2(GCancellable *cancellable = NULL;)
	CamelEasStore *eas_store;
	CamelEasStorePrivate *priv;
	CamelFolderInfo *fi = NULL;
	GSList *new_folders = NULL;

	eas_store = (CamelEasStore *) store;
	priv = eas_store->priv;

	g_mutex_lock (priv->get_finfo_lock);
	if (!(camel_offline_store_get_online (CAMEL_OFFLINE_STORE (store))
	      && EVO3_sync(camel_service_connect) ((CamelService *)store, error))) {
		g_mutex_unlock (priv->get_finfo_lock);
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

		g_mutex_unlock (priv->get_finfo_lock);
		return NULL;
	}
	eas_utils_sync_folders (eas_store, new_folders);
	g_mutex_unlock (priv->get_finfo_lock);

offline:
	fi = folder_info_from_store_summary ( (CamelEasStore *) store, top, flags, error);
	return fi;
}

static CamelFolderInfo*
eas_create_folder_sync (CamelStore *store,
		const gchar *parent_name,
		const gchar *folder_name,
		EVO3(GCancellable *cancellable,)
		GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Create folder not possible with ActiveSync"));
	return NULL;
}

static gboolean
eas_delete_folder_sync	(CamelStore *store,
			 const gchar *folder_name,
			 EVO3(GCancellable *cancellable,)
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
			EVO3(GCancellable *cancellable,)
			GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Rename folder not possible with ActiveSync"));
	return FALSE;
}

gchar *
eas_get_name (CamelService *service, gboolean brief)
{
	CamelURL *url = camel_service_get_camel_url (service);

	if (brief)
		return g_strdup_printf(_("Exchange ActiveSync server %s"),
				       url->host);
	else
		return g_strdup_printf(_("Exchange ActiveSync service for %s on %s"),
				       url->user, url->host);
}

EasEmailHandler *
camel_eas_store_get_handler (CamelEasStore *eas_store)
{
	return g_object_ref (eas_store->priv->handler);
}

#if EDS_CHECK_VERSION (2,33,0)
static CamelFolder *
eas_get_trash_folder_sync (CamelStore *store, EVO3(GCancellable *cancellable,) GError **error)
{
	return NULL;
}
#endif

static gboolean
eas_can_refresh_folder (CamelStore *store, CamelFolderInfo *info, GError **error)
{
	/* Skip unselectable folders from automatic refresh */
	if (info && (info->flags & CAMEL_FOLDER_NOSELECT) != 0) return FALSE;

	/* Delegate decision to parent class */
	return CAMEL_STORE_CLASS(camel_eas_store_parent_class)->can_refresh_folder (store, info, error) ||
		(camel_url_get_param (camel_service_get_camel_url(CAMEL_SERVICE(store)),
				      "check_all") != NULL);
}

gboolean
camel_eas_store_connected (CamelEasStore *eas_store, GError **error)
{

	if (!camel_offline_store_get_online (CAMEL_OFFLINE_STORE (eas_store))) {
		g_set_error (
			error, CAMEL_SERVICE_ERROR,
			CAMEL_SERVICE_ERROR_UNAVAILABLE,
			_("You must be working online to complete this operation"));
		return FALSE;
	}

	if (!EVO3_sync(camel_service_connect) ((CamelService *) eas_store, error))
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

	g_free (eas_store->priv->account_uid);

	g_free (eas_store->storage_path);
	g_mutex_free (eas_store->priv->get_finfo_lock);

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
#if !EDS_CHECK_VERSION(3,1,0)
	service_class->construct = eas_store_construct;
	service_class->get_path = eas_service_get_path;
#endif
	service_class->EVO3_sync(query_auth_types) = eas_store_query_auth_types_sync;
	service_class->get_name = eas_get_name;
	service_class->EVO3_sync(connect) = eas_connect_sync;
	service_class->EVO3_sync(disconnect) = eas_disconnect_sync;

	store_class = CAMEL_STORE_CLASS (class);
	store_class->hash_folder_name = eas_hash_folder_name;
	store_class->compare_folder_name = eas_compare_folder_name;
	store_class->EVO3_sync(get_folder) = eas_get_folder_sync;
	store_class->EVO3_sync(create_folder) = eas_create_folder_sync;
	store_class->EVO3_sync(delete_folder) = eas_delete_folder_sync;
	store_class->EVO3_sync(rename_folder) = eas_rename_folder_sync;
	store_class->EVO3_sync(get_folder_info) = eas_get_folder_info_sync;
	store_class->free_folder_info = camel_store_free_folder_info_full;

	EVO3(store_class->get_trash_folder_sync = eas_get_trash_folder_sync;)
	store_class->can_refresh_folder = eas_can_refresh_folder;
}

static void
camel_eas_store_init (CamelEasStore *eas_store)
{
	eas_store->priv =
		CAMEL_EAS_STORE_GET_PRIVATE (eas_store);

	eas_store->priv->account_uid = NULL;
	eas_store->priv->handler = NULL;
	eas_store->priv->last_refresh_time = time (NULL) - (FINFO_REFRESH_INTERVAL + 10);
	eas_store->priv->get_finfo_lock = g_mutex_new ();
}
