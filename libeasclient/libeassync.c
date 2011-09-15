/*
 * ActiveSync client library for calendar/addressbook synchronisation
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlsave.h>
#include <libedataserver/e-flag.h>

#include "../eas-daemon/src/activesyncd-common-defs.h"
#include "eas-folder.h"
#include "eas-mail-errors.h"
#include "libeassync.h"
#include "eas-item-info.h"
#include "eas-sync-errors.h"

#include "eas-logger.h"

G_DEFINE_TYPE (EasSyncHandler, eas_sync_handler, G_TYPE_OBJECT);

#define EAS_SYNC_HANDLER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_HANDLER, EasSyncHandlerPrivate))

GQuark
eas_sync_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0)) {
		const gchar *string = "eas-sync-error-quark";
		quark = g_quark_from_static_string (string);
	}

	return quark;
}


struct _EasSyncHandlerPrivate {
	DBusGConnection* bus;
	DBusGProxy *remoteEas;
	gchar* account_uid;     // TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
	GMainLoop* main_loop;

};

static gboolean
build_serialised_calendar_info_array (gchar ***serialised_cal_info_array, const GSList *cal_list, gboolean add_client_ids, GError **error);

// TODO - how much verification of args should happen

static void
eas_sync_handler_init (EasSyncHandler *cnc)
{
	EasSyncHandlerPrivate *priv = NULL;
	g_debug ("eas_sync_handler_init++");

	/* allocate internal structure */
	cnc->priv = priv = EAS_SYNC_HANDLER_PRIVATE (cnc);

	priv->remoteEas = NULL;
	priv->bus = NULL;
	priv->account_uid = NULL;
	priv->main_loop = NULL;
	cnc->priv = priv;
	g_debug ("eas_sync_handler_init--");
	srand (time (NULL) + rand());
}

static void
eas_sync_handler_dispose (GObject *object)
{
	EasSyncHandler *cnc = (EasSyncHandler *) object;
	EasSyncHandlerPrivate *priv = cnc->priv;

	if (priv->bus) {
		dbus_g_connection_unref (priv->bus);
		priv->bus = NULL;
	}
	G_OBJECT_CLASS (eas_sync_handler_parent_class)->dispose (object);
}

static void
eas_sync_handler_finalize (GObject *object)
{
	EasSyncHandler *cnc = EAS_SYNC_HANDLER (object);
	EasSyncHandlerPrivate *priv = cnc->priv;

	g_debug ("eas_sync_handler_finalize++");

	g_free (priv->account_uid);

	if (priv->main_loop) {
		g_main_loop_quit (priv->main_loop);
	}

	G_OBJECT_CLASS (eas_sync_handler_parent_class)->finalize (object);
	g_debug ("eas_sync_handler_finalize--");
}

static void
eas_sync_handler_class_init (EasSyncHandlerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_sync_handler_class_init++");

	g_type_class_add_private (klass, sizeof (EasSyncHandlerPrivate));

	object_class->finalize = eas_sync_handler_finalize;
	object_class->finalize = eas_sync_handler_dispose;
	g_debug ("eas_sync_handler_class_init--");
}

EasSyncHandler *
eas_sync_handler_new (const gchar* account_uid)
{
	GError* error = NULL;
	EasSyncHandler *object = NULL;

	g_type_init();
	dbus_g_thread_init();

	g_log_set_handler (G_LOG_DOMAIN,
			   G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL,
			   eas_logger,
			   NULL);

	g_debug ("eas_sync_handler_new++ : account_uid[%s]",
		 (account_uid ? account_uid : "NULL"));

	if (!account_uid) return NULL;

	object = g_object_new (EAS_TYPE_SYNC_HANDLER , NULL);

	if (object == NULL) {
		g_critical ("Error: Couldn't create sync handler");
		return NULL;
	}

	object->priv->main_loop = g_main_loop_new (NULL, TRUE);

	if (object->priv->main_loop == NULL) {
		g_object_unref (object);
		g_critical ("Error: Failed to create the mainloop");
		return NULL;
	}

	g_debug ("Connecting to Session D-Bus.");
	object->priv->bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (error != NULL) {
		g_object_unref (object);
		g_critical ("Error: Couldn't connect to the Session bus (%s) ", error->message);
		return NULL;
	}

	g_debug ("Creating a GLib proxy object for Eas.");
	object->priv->remoteEas =  dbus_g_proxy_new_for_name (object->priv->bus,
							      EAS_SERVICE_NAME,
							      EAS_SERVICE_SYNC_OBJECT_PATH,
							      EAS_SERVICE_SYNC_INTERFACE);
	if (object->priv->remoteEas == NULL) {
		g_critical ("Error: Couldn't create the proxy object");
		g_object_unref (object);
		return NULL;
	}

	dbus_g_proxy_set_default_timeout (object->priv->remoteEas, 1000000);
	object->priv->account_uid = g_strdup (account_uid);

	g_debug ("eas_sync_handler_new--");
	return object;
}

static void
free_string_array (gchar **array)
{
	guint i = 0;
	while (array && array[i]) {
		g_free (array[i]);
		i++;
	}
	g_free (array);

}

gboolean eas_sync_handler_get_items (EasSyncHandler* self,
				     const gchar *sync_key_in,
				     gchar **sync_key_out,
				     EasItemType type,
				     const gchar* folder_id,
				     GSList **items_created,
				     GSList **items_updated,
				     GSList **items_deleted,
				     gboolean *more_available,   // if there are more changes to sync (window_size exceeded)
				     GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy;
	gchar **created_item_array = NULL;
	gchar **deleted_item_array = NULL;
	gchar **updated_item_array = NULL;


	g_debug ("eas_sync_handler_get_calendar_items++ ");

	if (self == NULL) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("get_items requires a valid EasSyncHandler object"));
		return FALSE;
	} else if (items_created == NULL || items_updated == NULL || items_deleted == NULL) {

		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("get_items requires a valid  arguments items_created,items_updated & items_deleted"));
		return FALSE;

	}
	//if sync_key not set - assign to 0 to re-initiate sync relationship
	if (sync_key_in == NULL || (strlen (sync_key_in) <= 0)) {
		g_debug ("updating sync key to 0");
		sync_key_in = "0";
	}


	g_debug ("sync_key_in = %s", sync_key_in);
	g_debug ("eas_sync_handler_get_latest_items - dbus proxy ok");

	g_assert (g_slist_length (*items_created) == 0);
	g_assert (g_slist_length (*items_updated) == 0);
	g_assert (g_slist_length (*items_deleted) == 0);

	proxy = self->priv->remoteEas;
	// call DBus API
	ret = dbus_g_proxy_call (proxy, "get_latest_items", error,
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_UINT64, (guint64) type,
				 G_TYPE_STRING, folder_id,
				 G_TYPE_STRING, sync_key_in,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, sync_key_out,
				 G_TYPE_BOOLEAN, more_available,
				 G_TYPE_STRV, &created_item_array,
				 G_TYPE_STRV, &deleted_item_array,
				 G_TYPE_STRV, &updated_item_array,
				 G_TYPE_INVALID);

	g_debug ("eas_sync_handler_get_latest_items - dbus proxy called");

	if (ret) {
		guint i = 0;
		g_debug ("get_latest_items called successfully");
		while (created_item_array[i]) {
			EasItemInfo *cal = eas_item_info_new ();
			g_debug ("created item = %s", created_item_array[i]);
			eas_item_info_deserialise (cal, created_item_array[i]);
			g_debug ("created item server id = %s", cal->server_id);
			*items_created = g_slist_append (*items_created, cal);
			i++;
		}
		i = 0;
		while (updated_item_array[i]) {
			EasItemInfo *cal = eas_item_info_new ();
			g_debug ("created item = %s", updated_item_array[i]);
			eas_item_info_deserialise (cal, updated_item_array[i]);
			g_debug ("created item server id = %s", cal->server_id);
			*items_updated = g_slist_append (*items_updated, cal);
			i++;
		}
		i = 0;
		while (deleted_item_array[i]) {
			g_debug ("deleted item = %s", deleted_item_array[i]);
			*items_deleted = g_slist_append (*items_deleted, g_strdup (deleted_item_array[i]));
			i++;
		}

	}

	free_string_array (created_item_array);
	free_string_array (updated_item_array);
	free_string_array (deleted_item_array);

	if (!ret) { // failed - cleanup lists
		if (*items_created) {
			g_slist_foreach (*items_created, (GFunc) g_object_unref, NULL);
			g_slist_free (*items_created);
			*items_created = NULL;
		}
		if (*items_updated) {
			g_slist_foreach (*items_updated, (GFunc) g_object_unref, NULL);
			g_slist_free (*items_updated);
			*items_updated = NULL;
		}
		if (*items_deleted) {
			g_slist_foreach (*items_deleted, (GFunc) g_object_unref, NULL);
			g_slist_free (*items_deleted);
			*items_deleted = NULL;
		}
	} else {
		g_debug ("sync_key = %s", *sync_key_out);
	}

	g_debug ("eas_sync_handler_get_items--");
	return ret;
}

gboolean
eas_sync_handler_delete_items (EasSyncHandler* self,
			       const gchar *sync_key_in,
			       gchar **sync_key_out,
			       EasItemType type,
			       const gchar* folder_id,
			       GSList *items_deleted,
			       GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = NULL;

	// Build string array from items_deleted GSList
	guint list_length = g_slist_length ( (GSList*) items_deleted);
	gchar **deleted_items_array = NULL;
	int loop = 0;

	g_debug ("eas_sync_handler_delete_items++");
	if (self == NULL) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("delete_items requires a valid EasSyncHandler object"));
		return FALSE;
	} else if (sync_key_in == NULL || (strlen (sync_key_in) <= 0) || !g_strcmp0 (sync_key_in, "0")) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("delete_items requires a valid sync key"));
		return FALSE;
	}

	proxy = self->priv->remoteEas;

	deleted_items_array = g_malloc0 ( (list_length + 1) * sizeof (gchar*));
	if (!deleted_items_array) {
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		return FALSE;
	}

	for (; loop < list_length; ++loop) {
		deleted_items_array[loop] = g_strdup (g_slist_nth_data ( (GSList*) items_deleted, loop));
		g_debug ("Deleted Id: [%s]", deleted_items_array[loop]);
	}


	g_debug ("eas_sync_handler_delete_items - dbus proxy ok");

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "delete_items", error,
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_UINT64, (guint64) type,
				 G_TYPE_STRING, folder_id,
				 G_TYPE_STRING, sync_key_in,
				 G_TYPE_STRV, deleted_items_array,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, sync_key_out,
				 G_TYPE_INVALID);

	g_debug ("eas_sync_handler_delete_items - dbus proxy called");

	if (ret) {
		g_debug ("delete_items called successfully");
	}

	g_debug ("eas_sync_handler_delete_items--");
	return ret;
}

gboolean
eas_sync_handler_update_items (EasSyncHandler* self,
			       const gchar *sync_key_in,
			       gchar **sync_key_out,
			       EasItemType type,
			       const gchar* folder_id,
			       GSList *items_updated,
			       GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy;
	gchar **updated_item_array = NULL;

	g_debug ("eas_sync_handler_update_items++");
	if (self == NULL) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("update_items requires a valid EasSyncHandler object"));
		return FALSE;
	} else if (sync_key_in == NULL || (strlen (sync_key_in) <= 0) || !g_strcmp0 (sync_key_in, "0")) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("update_items requires a valid sync key"));
		return FALSE;
	}


	g_debug ("eas_sync_handler_update_items - dbus proxy ok");

	proxy = self->priv->remoteEas;

	build_serialised_calendar_info_array (&updated_item_array, items_updated, FALSE, error);

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "update_items", error,
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_UINT64, (guint64) type,
				 G_TYPE_STRING, folder_id,
				 G_TYPE_STRING, sync_key_in,
				 G_TYPE_STRV, updated_item_array,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, sync_key_out,
				 G_TYPE_INVALID);

	g_debug ("eas_sync_handler_update_items - dbus proxy called");

	if (ret) {
		g_debug ("update_calendar_items called successfully");
	}

	g_debug ("eas_sync_handler_update_items--");
	return ret;
}

// allocates an array of ptrs to strings and the strings it points to and populates each string with serialised folder details
// null terminates the array
static gboolean
build_serialised_calendar_info_array (gchar ***serialised_cal_info_array, const GSList *cal_list, gboolean add_client_ids, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	GSList *l = (GSList*) cal_list;
	guint array_len = g_slist_length ( (GSList*) cal_list) + 1; //cast away const to avoid warning. +1 to allow terminating null

	g_debug ("build cal arrays++");
	g_assert (serialised_cal_info_array);
	g_assert (*serialised_cal_info_array == NULL);

	*serialised_cal_info_array = g_malloc0 (array_len * sizeof (gchar*));

	for (i = 0; i < array_len - 1; i++) {
		EasItemInfo *calInfo = l->data;
		gchar *tstring = NULL;
		g_assert (l != NULL);
		//if we're adding data, and it has no client id - make one up
		if (add_client_ids && calInfo->client_id == NULL) {
			gchar client_id[21];
			guint random_num;
			random_num = rand();
			snprintf (client_id, sizeof (client_id) / sizeof (client_id[0]), "%d", random_num);
			calInfo->client_id = g_strdup (client_id);
		}
		eas_item_info_serialise (calInfo, &tstring);
		(*serialised_cal_info_array) [i] = tstring;
		l = g_slist_next (l);
	}

	return ret;
}

gboolean
eas_sync_handler_add_items (EasSyncHandler* self,
			    const gchar *sync_key_in,
			    gchar **sync_key_out,
			    EasItemType type,
			    const gchar* folder_id,
			    GSList *items_added,
			    GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = NULL;
	gchar **added_item_array = NULL;
	gchar **created_item_array = NULL;

	g_debug ("eas_sync_handler_add_items++");
	if (self == NULL) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("add_items requires a valid EasSyncHandler object"));
		return FALSE;
	} else if (sync_key_in == NULL || (strlen (sync_key_in) <= 0) || !g_strcmp0 (sync_key_in, "0")) {
		g_set_error (error, EAS_SYNC_ERROR,
			     EAS_SYNC_ERROR_BADARG,
			     ("add_items requires a valid sync key"));
		return FALSE;
	}


	proxy = self->priv->remoteEas;
	build_serialised_calendar_info_array (&added_item_array, items_added, TRUE, error);

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "add_items", error,
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_UINT64, (guint64) type,
				 G_TYPE_STRING, folder_id,
				 G_TYPE_STRING, sync_key_in,
				 G_TYPE_STRV, added_item_array,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, sync_key_out,
				 G_TYPE_STRV, &created_item_array,
				 G_TYPE_INVALID);

	g_debug ("eas_sync_handler_add_items - dbus proxy called");

	if (error && *error) {
		g_warning (" Error: %s", (*error)->message);
	}

	if (ret) {
		guint i = 0;
		guint length = g_slist_length (items_added);
		g_debug ("add_calendar_items called successfully");
		while (created_item_array[i] && i < length) {
			EasItemInfo *cal = eas_item_info_new ();
			EasItemInfo *updated = g_slist_nth (items_added, i)->data;
			g_debug ("created item = %s", created_item_array[i]);
			eas_item_info_deserialise (cal, created_item_array[i]);
			g_debug ("created item server id = %s", cal->server_id);
			updated->server_id = g_strdup (cal->server_id);
			updated->status = g_strdup (cal->status);
			g_debug ("created updated server id in list = %s", cal->server_id);
			i++;
		}
		if (i == length && created_item_array[i]) {
			g_debug ("added list is not the same length as input list - problem?");
		}
	}

	g_debug ("eas_sync_handler_add_items--");
	return ret;
}

gboolean
eas_sync_handler_fetch_item (EasSyncHandler* self,
			     const gchar *folder_id,
			     const gchar *server_id,
			     EasItemInfo* item,
			     EasItemType type,
			     GError **error)
{
	gboolean ret = TRUE;
	EasSyncHandlerPrivate *priv = self->priv;
	DBusGProxyCall *call;
	gchar* flatitem = NULL;

	g_debug ("eas_sync_handler_fetch_item++");
	g_assert (self);
	g_assert (server_id);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);


	call = dbus_g_proxy_begin_call (priv->remoteEas,
					"fetch_item",
					NULL,
					NULL, 							// userdata passed to callback
					NULL, 							// destroy notification
					G_TYPE_STRING, priv->account_uid,
					G_TYPE_STRING, folder_id,
					G_TYPE_STRING, server_id,
					G_TYPE_UINT64, (guint64) type,
					G_TYPE_INVALID);


	// blocks until results are available:
	ret = dbus_g_proxy_end_call (priv->remoteEas,
				     call,
				     error,
				     G_TYPE_STRING, &flatitem,
				     G_TYPE_INVALID);


	eas_item_info_deserialise (item, flatitem);

	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_mail_handler_fetch_email_body--");
	return ret;
}