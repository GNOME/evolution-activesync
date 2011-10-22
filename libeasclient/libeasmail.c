/*
 * ActiveSync client library for email access
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
#include <glib/gstdio.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlsave.h>
#include <libedataserver/e-flag.h>

#include "eas-errors.h"
#include "../eas-daemon/src/activesyncd-common-defs.h"
#include "libeassync.h"
#include "libeasmail.h"
#include "eas-mail-client-stub.h"
#include "eas-folder.h"
#include "eas-mail-errors.h"
#include "eas-logger.h"
#include "eas-marshal.h"

G_DEFINE_TYPE (EasEmailHandler, eas_mail_handler, G_TYPE_OBJECT);

#define EAS_EMAIL_HANDLER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_EMAIL_HANDLER, EasEmailHandlerPrivate))

GQuark
eas_mail_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0)) {
		const gchar *string = "eas-mail-error-quark";
		quark = g_quark_from_static_string (string);
	}

	return quark;
}

const gchar *updated_id_separator = ",";

static GStaticMutex progress_table = G_STATIC_MUTEX_INIT;

struct _EasEmailHandlerPrivate {
	/* New GIO/GDBus code: */
	GDBusConnection *connection;
	/* Obsolescent dbus-glib bits: */
	DBusGConnection *bus;
	DBusGProxy *remoteEas;
	DBusGProxy *remoteCommonEas;

	gchar* account_uid;     // TODO - is it appropriate to have a dbus proxy per account if we have multiple accounts making requests at same time?
	GHashTable *email_progress_fns_table;	// hashtable of request progress functions
	guint next_request_id;			// request id to be used for next dbus call

};


struct _EasProgressCallbackInfo {
	EasProgressFn progress_fn;
	gpointer progress_data;
	guint percent_last_sent;

	guint handler_id;	//
	GCancellable cancellable;	//

};

typedef struct _EasProgressCallbackInfo EasProgressCallbackInfo;

struct _EasCancelInfo {
	EasEmailHandler *handler;	// the email handler in use
	guint request_id;			// request id to cancel
};

typedef struct _EasCancelInfo EasCancelInfo;

// fwd declarations:
static void progress_signal_handler (DBusGProxy * proxy,
				     guint request_id,
				     guint percent,
				     gpointer user_data);

static void eas_mail_handler_cancel_common_request (GCancellable *cancellable,
						    gpointer user_data);

static void eas_mail_handler_cancel_mail_request (GCancellable *cancellable,
						  gpointer user_data);

// TODO - how much verification of args should happen??

static void
eas_mail_handler_init (EasEmailHandler *cnc)
{
	EasEmailHandlerPrivate *priv;
	g_debug ("eas_mail_handler_init++");

	/* allocate internal structure */
	cnc->priv = priv = EAS_EMAIL_HANDLER_PRIVATE (cnc);

	priv->connection = NULL;
	priv->remoteEas = NULL;
	priv->remoteCommonEas = NULL;
	priv->bus = NULL;
	priv->account_uid = NULL;
	priv->next_request_id = 1;
	priv->email_progress_fns_table = NULL;
	g_debug ("eas_mail_handler_init--");
}

static void
eas_mail_handler_dispose (GObject *object)
{
	EasEmailHandler *cnc = EAS_EMAIL_HANDLER (object);
	EasEmailHandlerPrivate *priv;
	priv = cnc->priv;

	if (priv->bus) {
		dbus_g_connection_unref (priv->bus);
		priv->bus = NULL;
	}

	G_OBJECT_CLASS (eas_mail_handler_parent_class)->dispose (object);
}

static void
eas_mail_handler_finalize (GObject *object)
{
	EasEmailHandler *cnc = EAS_EMAIL_HANDLER (object);
	EasEmailHandlerPrivate *priv;
	g_debug ("eas_mail_handler_finalize++");

	priv = cnc->priv;

	// de-register for progress signals
	if (priv->remoteEas) {
		dbus_g_proxy_disconnect_signal (priv->remoteEas,
						EAS_MAIL_SIGNAL_PROGRESS,
						G_CALLBACK (progress_signal_handler),
						cnc);
		g_object_unref (priv->remoteEas);
	}
	if (priv->remoteCommonEas) {
		dbus_g_proxy_disconnect_signal (priv->remoteCommonEas,
						EAS_MAIL_SIGNAL_PROGRESS,
						G_CALLBACK (progress_signal_handler),
						cnc);
		g_object_unref (priv->remoteCommonEas);
	}

	if (priv->connection)
		g_object_unref (priv->connection);

	g_free (priv->account_uid);

	// free the hashtable
	if (priv->email_progress_fns_table) {
		g_hash_table_remove_all (priv->email_progress_fns_table);
	}
	// nothing to do to 'free' proxy

	G_OBJECT_CLASS (eas_mail_handler_parent_class)->finalize (object);
	g_debug ("eas_mail_handler_finalize--");
}

static void
eas_mail_handler_class_init (EasEmailHandlerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_mail_handler_class_init++");

	g_type_class_add_private (klass, sizeof (EasEmailHandlerPrivate));

	object_class->finalize = eas_mail_handler_finalize;
	object_class->dispose = eas_mail_handler_dispose;
	g_debug ("eas_mail_handler_class_init--");
}


static gboolean
eas_mail_add_progress_info_to_table (EasEmailHandler* self, guint request_id, EasProgressFn progress_fn, gpointer progress_data, GError **error)
{
	gboolean ret = TRUE;
	EasEmailHandlerPrivate *priv = self->priv;
	EasProgressCallbackInfo *progress_info = g_malloc0 (sizeof (EasProgressCallbackInfo));

	if (!progress_info) {
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		ret = FALSE;
		goto finish;
	}

	// add progress fn/data structure to hash table
	progress_info->progress_fn = progress_fn;
	progress_info->progress_data = progress_data;
	progress_info->percent_last_sent = 0;

	g_static_mutex_lock (&progress_table);
	if (priv->email_progress_fns_table == NULL) {
		priv->email_progress_fns_table = g_hash_table_new_full (NULL, NULL, NULL, g_free);
	}
	g_debug ("insert progress function into table");
	g_hash_table_insert (priv->email_progress_fns_table, GUINT_TO_POINTER (request_id), progress_info);
	g_static_mutex_unlock (&progress_table);
finish:
	return ret;
}

EasEmailHandler *
eas_mail_handler_new (const char* account_uid, GError **error)
{
	EasEmailHandler *object = NULL;
	EasEmailHandlerPrivate *priv = NULL;

	g_type_init();

	dbus_g_thread_init();

	/* Ick. See https://bugzilla.gnome.org/show_bug.cgi?id=662396 */
	eas_connection_error_quark();

	g_log_set_handler (G_LOG_DOMAIN,
			   G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL,
			   eas_logger,
			   NULL);

	g_debug ("eas_mail_handler_new++ : account_uid[%s]", (account_uid ? account_uid : "NULL"));

	if (!account_uid) {
		g_set_error (error, EAS_MAIL_ERROR, EAS_MAIL_ERROR_UNKNOWN,
			     "No account UID specified");
		return NULL;
	}

	object = g_object_new (EAS_TYPE_EMAIL_HANDLER, NULL);

	if (object == NULL) {
		g_set_error (error, EAS_MAIL_ERROR, EAS_MAIL_ERROR_UNKNOWN,
			     "Could not create email handler object");
		g_warning ("Error: Couldn't create mail");
		g_debug ("eas_mail_handler_new--");
		return NULL;
	}
	priv = object->priv;

	g_debug ("Connecting to Session D-Bus.");
	priv->bus = dbus_g_bus_get (DBUS_BUS_SESSION, error);
	if (priv->bus == NULL) {
		g_warning ("Error: Couldn't connect to the Session bus (%s) ", error ? (*error)->message : "<discarded error>");
		return NULL;
	}

	g_debug ("Creating a GLib proxy object for Eas.");
	priv->remoteEas =  dbus_g_proxy_new_for_name (priv->bus,
						      EAS_SERVICE_NAME,
						      EAS_SERVICE_MAIL_OBJECT_PATH,
						      EAS_SERVICE_MAIL_INTERFACE);
	if (priv->remoteEas == NULL) {
		g_set_error (error, EAS_MAIL_ERROR, EAS_MAIL_ERROR_UNKNOWN,
			     "ailed to create proxy for mail interface");
		g_warning ("Error: Couldn't create the proxy object");
		return NULL;
	}

	g_debug ("Creating a GLib proxy object for Eas.");
	priv->remoteCommonEas =  dbus_g_proxy_new_for_name (priv->bus,
							    EAS_SERVICE_NAME,
							    EAS_SERVICE_COMMON_OBJECT_PATH,
							    EAS_SERVICE_COMMON_INTERFACE);
	if (priv->remoteCommonEas == NULL) {
		g_set_error (error, EAS_MAIL_ERROR, EAS_MAIL_ERROR_UNKNOWN,
			     "Failed to create proxy for common interface");
		g_warning ("Error: Couldn't create the proxy object for common");
		return NULL;
	}

	dbus_g_proxy_set_default_timeout (priv->remoteEas, 1000000);
	dbus_g_proxy_set_default_timeout (priv->remoteCommonEas, 1000000);
	priv->account_uid = g_strdup (account_uid);

	priv->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);
	if (!priv->connection)
		return NULL;

	/* Register dbus signal marshaller */
	dbus_g_object_register_marshaller (eas_marshal_VOID__UINT_UINT,
					   G_TYPE_NONE, G_TYPE_UINT, G_TYPE_UINT,
					   G_TYPE_INVALID);

	g_debug ("register as observer of %s signal", EAS_MAIL_SIGNAL_PROGRESS);
	// progress signal setup:
	dbus_g_proxy_add_signal (priv->remoteEas,
				 EAS_MAIL_SIGNAL_PROGRESS,
				 G_TYPE_UINT,	// request id
				 G_TYPE_UINT,	// percent
				 G_TYPE_INVALID);
	// progress signal setup:
	dbus_g_proxy_add_signal (priv->remoteCommonEas,
				 EAS_MAIL_SIGNAL_PROGRESS,
				 G_TYPE_UINT,	// request id
				 G_TYPE_UINT,	// percent
				 G_TYPE_INVALID);

	// register for progress signals from mail and common interfaces
	dbus_g_proxy_connect_signal (priv->remoteEas,
				     EAS_MAIL_SIGNAL_PROGRESS,
				     G_CALLBACK (progress_signal_handler),		// callback when signal emitted
				     object,													// userdata passed to above cb
				     NULL);

	dbus_g_proxy_connect_signal (priv->remoteCommonEas,
				     EAS_MAIL_SIGNAL_PROGRESS,
				     G_CALLBACK (progress_signal_handler),		// callback when signal emitted
				     object,									// userdata passed to above cb
				     NULL);

	g_debug ("eas_mail_handler_new--");
	return object;
}

/* g_dbus_connection_call() doesn't let us see the serial#, and ideally
   we want that in order to let the progress messages use it rather than
   having to make up a new handle for ourselves. So we do it ourselves. */

struct _eas_call_data {
	GAsyncResult *result;
	GMainLoop *loop;
	gboolean cancelled;
};

static void
_call_done (GObject *conn, GAsyncResult *result, gpointer _call)
{
	struct _eas_call_data *call = _call;

	call->result = g_object_ref (result);
	g_main_loop_quit (call->loop);
}

static void
_call_cancel (GCancellable *cancellable, gpointer _call)
{
	struct _eas_call_data *call = _call;

	/* We can't send the DBus message from here; the GDBusConnection will
	   deadlock itself in g_cancellable_disconnect, because it wants its
	   *own* cancel handler to complete but ends up waiting for *this* one,
	   which in turn is waiting for the connection lock. */
	call->cancelled = TRUE;
}

static gboolean
eas_gdbus_mail_call (EasEmailHandler *self, const gchar *method,
		     EasProgressFn progress_fn, gpointer progress_data,
		     const gchar *in_params, const gchar *out_params,
		     GCancellable *cancellable, GError **error, ...)
{
	GDBusMessage *message;
	struct _eas_call_data call;
	GMainContext *ctxt;
	GDBusMessage *reply;
	GVariant *v = NULL;
	va_list ap;
	gboolean success = FALSE;
	guint cancel_handler_id;
	guint32 serial = 0;
	gchar *out_params_type = (gchar *) out_params;

	va_start (ap, error);

	message = g_dbus_message_new_method_call (EAS_SERVICE_NAME,
						  EAS_SERVICE_MAIL_OBJECT_PATH,
						  EAS_SERVICE_MAIL_INTERFACE,
						  method);

	v = g_variant_new_va (in_params, NULL, &ap);
	g_dbus_message_set_body (message, v);

	call.cancelled = FALSE;
	call.result = NULL;
	ctxt = g_main_context_new ();
	call.loop = g_main_loop_new (ctxt, FALSE);

	g_main_context_push_thread_default (ctxt);

	g_dbus_connection_send_message_with_reply (self->priv->connection,
						   message,
						   G_DBUS_SEND_MESSAGE_FLAGS_NONE,
						   1000000,
						   &serial,
						   cancellable,
						   _call_done,
						   (gpointer) &call);
	if (cancellable)
		cancel_handler_id = g_cancellable_connect (cancellable,
							  G_CALLBACK (_call_cancel),
							  (gpointer) &call, NULL);

	/* Ignore error; it's not the end of the world if progress info
	   is lost, and it should never happen anyway */
	eas_mail_add_progress_info_to_table (self, serial,
					     progress_fn, progress_data, NULL);

	g_main_loop_run (call.loop);

	reply = g_dbus_connection_send_message_with_reply_finish(self->priv->connection,
								 call.result,
								 error);
	if (cancellable)
		g_cancellable_disconnect (cancellable, cancel_handler_id);

	if (call.cancelled) {
		message = g_dbus_message_new_method_call (EAS_SERVICE_NAME,
							  EAS_SERVICE_COMMON_OBJECT_PATH,
							  EAS_SERVICE_COMMON_INTERFACE,
							  "cancel_request");
		g_dbus_message_set_body (message,
					 g_variant_new ("(su)",
							self->priv->account_uid,
							serial));

		g_dbus_connection_send_message (self->priv->connection,
						message,
						G_DBUS_SEND_MESSAGE_FLAGS_NONE,
						NULL, NULL);
	}

	g_main_context_pop_thread_default (ctxt);
	g_main_context_unref (ctxt);
	g_main_loop_unref (call.loop);
	g_object_unref (call.result);

	if (!reply)
		goto out_no_reply;

	/* g_variant_is_of_type() will fail to match a DBus return
	   of (sas) with a format string of (s^as), where the ^ is
	   required to make it convert to a strv instead of something
	   more complicated. So we remove all ^ characters from the
	   string that we show to g_variant_is_of_type(). Ick. */
	if (out_params && strchr (out_params, '^')) {
		gchar *x, *y;

		out_params_type = g_strdup (out_params);

		x = y = strchr (out_params_type, '^');
		y++;

		while (*y) {
			if (*y == '^')
				y++;
			else
				*(x++) = *(y++);
		}
		*x = 0;
	}
	switch (g_dbus_message_get_message_type (reply)) {
	case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
		/* An empty (successful) response will give a NULL GVariant here */
		v = g_dbus_message_get_body (reply);
		if (!out_params) {
			if (v)
				goto inval;
			else {
				success = TRUE;
				break;
			}
		}
		if (!g_variant_is_of_type (v, G_VARIANT_TYPE (out_params_type))) {
		inval:
			g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
				     "ActiveSync DBus call returned invalid response type %s",
				     v?g_variant_get_type_string (v):"()");
			goto out;
			g_object_unref (reply);
		}
		g_variant_get_va (v, out_params, NULL, &ap);
		success = TRUE;
		break;

	case G_DBUS_MESSAGE_TYPE_ERROR:
		g_dbus_message_to_gerror (reply, error);
		break;

	default:
		g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
			     "EAS DBus call returned weird message type %d",
			     g_dbus_message_get_message_type (reply));
		break;
	}

 out:
	if (out_params_type != out_params)
		g_free (out_params_type);

	g_object_unref (reply);
 out_no_reply:
	if (serial)
		g_hash_table_remove (self->priv->email_progress_fns_table,
				     GUINT_TO_POINTER (serial));

	va_end (ap);
	return success;
}

// takes an NULL terminated array of serialised folders and creates a list of EasFolder objects
static gboolean
build_folder_list (const gchar **serialised_folder_array, GSList **folder_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;

	g_debug ("build_folder_list++");
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert (folder_list);
	g_assert (*folder_list == NULL);

	while (serialised_folder_array[i]) {
		EasFolder *folder = eas_folder_new();
		if (folder) {
			*folder_list = g_slist_append (*folder_list, folder);   // add it to the list first to aid cleanup
			if (!folder_list) {
				g_free (folder);
				ret = FALSE;
				goto cleanup;
			}
			if (!eas_folder_deserialise (folder, serialised_folder_array[i])) {
				ret = FALSE;
				goto cleanup;
			}
		} else {
			ret = FALSE;
			goto cleanup;
		}
		i++;
	}

cleanup:
	if (!ret) {
		// set the error
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		// clean up on error
		g_slist_foreach (*folder_list, (GFunc) g_free, NULL);
		g_slist_free (*folder_list);
		*folder_list = NULL;
	}

	g_debug ("list has %d items", g_slist_length (*folder_list));
	g_debug ("build_folder_list++");
	return ret;
}


// takes an NULL terminated array of serialised emailinfos and creates a list of EasEmailInfo objects
static gboolean
build_emailinfo_list (const gchar **serialised_emailinfo_array, GSList **emailinfo_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	g_debug ("build_emailinfo_list++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_assert (*emailinfo_list == NULL);

	while (serialised_emailinfo_array[i]) {
		EasEmailInfo *emailinfo = eas_email_info_new ();
		if (emailinfo) {
			*emailinfo_list = g_slist_append (*emailinfo_list, emailinfo);
			if (!*emailinfo_list) {
				g_free (emailinfo);
				ret = FALSE;
				goto cleanup;
			}
			if (!eas_email_info_deserialise (emailinfo, serialised_emailinfo_array[i])) {
				ret = FALSE;
				goto cleanup;
			}
		} else {
			ret = FALSE;
			goto cleanup;
		}
		i++;
	}

cleanup:
	if (!ret) {
		// set the error
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		// clean up on error
		g_slist_foreach (*emailinfo_list, (GFunc) g_object_unref, NULL);
		g_slist_free (*emailinfo_list);
		*emailinfo_list = NULL;
	}

	g_debug ("build_emailinfo_list--");
	return ret;
}

void
eas_updatedid_free (EasIdUpdate* updated_id)
{
	g_debug ("eas_updatedid_free++");
	g_free (updated_id->status);
	g_free (updated_id->dest_id);
	g_free (updated_id->src_id);
	g_free (updated_id);
	g_debug ("eas_updatedid_free--");
}

/*
take the contents of the structure and turn it into a null terminated string
*/
gboolean
eas_updatedid_serialise (const EasIdUpdate* updated_id, gchar **result)
{
	gboolean ret = TRUE;

	const gchar *strings[4];

	g_debug ("eas_updatedid_serialise++");

	strings[0] = updated_id->src_id;
	strings[1] = (updated_id->dest_id ? : "");	// may be null
	strings[2] = (updated_id->status ? : "");	// may be null
	strings[3] = NULL;

	*result = g_strjoinv (updated_id_separator, (gchar **) strings);

	if (*result == NULL) {
		ret = FALSE;
	}
	g_debug ("eas_updatedid_serialise--");
	return ret;
}

/*
populate the object from a string
*/
static gboolean
eas_updatedid_deserialise (EasIdUpdate *updated_id, const gchar* data)
{
	gboolean ret = FALSE;
	gchar **strv;

	g_debug ("eas_updatedid_deserialise++");
	g_assert (updated_id);
	g_assert (data);
	g_assert (updated_id->dest_id == NULL);
	g_assert (updated_id->src_id == NULL);

	strv = g_strsplit (data, updated_id_separator, 0);
	if (!strv || g_strv_length (strv) > 3) {
		g_warning ("Received invalid updateid: '%s'", data);
		g_strfreev (strv);
		goto out;
	}

	ret = TRUE;
	updated_id->src_id = strv[0];
	/* This one might be NULL; that's OK */
	updated_id->dest_id = strv[1];
	/* This one might be NULL; that's OK */
	updated_id->status = strv[2];

	g_free (strv);

out:
	g_debug ("eas_updatedid_deserialise++");
	return ret;
}

// converts an NULL terminated array of serialised EasIdUpdates to a list
static gboolean
build_easidupdates_list (const gchar **updated_ids_array, GSList **updated_ids_list, GError **error)
{
	gboolean ret = TRUE;
	guint i = 0;
	g_debug ("build_easidupdates_list++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_assert (*updated_ids_list == 0);

	while (updated_ids_array[i]) {
		EasIdUpdate *updated_id = g_malloc0 (sizeof (EasIdUpdate));
		if (updated_id) {
			*updated_ids_list = g_slist_append (*updated_ids_list, updated_id);
			if (!*updated_ids_list) {
				g_free (updated_id);
				ret = FALSE;
				goto cleanup;
			}
			if (!eas_updatedid_deserialise (updated_id, updated_ids_array[i])) {
				ret = FALSE;
				goto cleanup;
			}
		} else {
			ret = FALSE;
			goto cleanup;
		}
		i++;
	}

cleanup:
	if (!ret) {
		// set the error
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		// clean up on error
		g_slist_foreach (*updated_ids_list, (GFunc) g_free, NULL);
		g_slist_free (*updated_ids_list);
		*updated_ids_list = NULL;
	}

	g_debug ("build_easidupdates_list++");
	return ret;
}

// TODO remove and use..._strfreev?
static void
free_string_array (gchar **array)
{
	guint i = 0;

	if (array == NULL)
		return;

	while (array[i]) {
		g_free (array[i]);
		i++;
	}
	g_free (array);

}

gboolean eas_mail_handler_get_item_estimate (EasEmailHandler* self,
					     const gchar *sync_key,
					     const gchar *folder_id, // folder to sync
					     guint *estimate,
					     GError **error)
{
	return eas_gdbus_mail_call (self, "get_item_estimate",
				    NULL, NULL, "(sss)", "(u)",
				    NULL, error,
				    self->priv->account_uid, sync_key, folder_id,
				    estimate);
}

gboolean
eas_mail_handler_get_folder_list (EasEmailHandler *self,
				  gboolean force_refresh,
				  GSList **folders,
				  GCancellable *cancellable,
				  GError **error)
{
	EasEmailHandlerPrivate *priv = self->priv;
	gboolean ret = TRUE;
	DBusGProxy *proxy = priv->remoteCommonEas;
	gchar **folder_array = NULL;
	guint cancel_handler_id;
	guint request_id = priv->next_request_id++;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (self->priv->account_uid ? self->priv->account_uid : "NULL"));

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert (self);
	g_assert (*folders == NULL);

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_common_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "get_folders",
				 error,
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_BOOLEAN, force_refresh,
				 G_TYPE_INVALID,
				 G_TYPE_STRV, &folder_array,
				 G_TYPE_INVALID);

	g_debug ("%s - dbus proxy called", __func__);

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	if (!ret) {
		if (error && *error) {
			g_warning ("[%s][%d][%s]",
				   g_quark_to_string ( (*error)->domain),
				   (*error)->code,
				   (*error)->message);
		}
		g_warning ("DBus dbus_g_proxy_call failed");
		goto cleanup;
	}

	g_debug ("%s called successfully", __func__);

	// get 3 arrays of strings of 'serialised' EasFolders, convert to EasFolder lists:
	ret = build_folder_list ( (const gchar **) folder_array, folders, error);

cleanup:
	free_string_array (folder_array);

	if (!ret) { // failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		if (error) {
			g_warning (" Error: %s", (*error)->message);
		}
		g_debug ("%s failure - cleanup lists", __func__);
		g_slist_foreach (*folders, (GFunc) g_free, NULL);
		g_free (*folders);
		*folders = NULL;
	}

	g_debug ("%s--", __func__);
	return ret;
}


gboolean
eas_mail_handler_get_provision_list (EasEmailHandler *self,
				     gchar** tid,
				     gchar** tid_status,
				     EasProvisionList **provision_list,
				     GCancellable *cancellable,
				     GError **error)
{
	EasEmailHandlerPrivate *priv = EAS_EMAIL_HANDLER_PRIVATE (self);
	gboolean ret = TRUE;
	DBusGProxy *proxy = priv->remoteCommonEas;
	gchar* _provision_list_buffer = NULL;
	gchar* _tid = NULL;
	gchar* _tid_status = NULL;
	guint cancel_handler_id;
	guint request_id = priv->next_request_id++;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (priv->account_uid ? priv->account_uid : "NULL"));

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert (self);
	g_assert (*provision_list == NULL);

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_common_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "get_provision_list",
				 error,
				 G_TYPE_STRING, priv->account_uid,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, &_tid,
				 G_TYPE_STRING, &_tid_status,
				 G_TYPE_STRING, &_provision_list_buffer,
				 G_TYPE_INVALID);

	g_debug ("%s - dbus proxy called", __func__);

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	if (!ret) {
		if (error && *error) {
			g_warning ("[%s][%d][%s]",
				   g_quark_to_string ( (*error)->domain),
				   (*error)->code,
				   (*error)->message);
		}
		g_warning ("DBus dbus_g_proxy_call failed");
		goto cleanup;
	}

	g_debug ("%s called successfully", __func__);

	*tid = _tid; // full transfer
	*tid_status = _tid_status; // full transfer

	// Build the provision list
	*provision_list = eas_provision_list_new();
	ret = eas_provision_list_deserialise (*provision_list, _provision_list_buffer);
	if (!ret) {
		// TODO: error
	}

cleanup:
	g_free (_provision_list_buffer);

	if (!ret) { // failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		if (error) {
			g_warning (" Error: %s", (*error)->message);
		}
		g_debug ("%s failure - cleanup lists", __func__);
		g_object_unref (*provision_list);
		*provision_list = NULL;
	}

	g_debug ("%s--", __func__);
	return ret;
}

gboolean
eas_mail_handler_autodiscover (EasEmailHandler *self,
			       const gchar* email,
			       const gchar* username,
			       gchar** uri,
			       GCancellable *cancellable,
			       GError **error)
{
	EasEmailHandlerPrivate *priv = EAS_EMAIL_HANDLER_PRIVATE (self);
	gboolean ret = TRUE;
	DBusGProxy *proxy = priv->remoteCommonEas;
	guint cancel_handler_id;
	gchar* _server_uri = NULL;
	guint request_id = priv->next_request_id++;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (priv->account_uid ? priv->account_uid : "NULL"));

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert (self);

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_common_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "autodiscover",
				 error,
				 G_TYPE_STRING, email,
				 G_TYPE_STRING, username,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, &_server_uri,
				 G_TYPE_INVALID);

	g_debug ("%s - dbus proxy called", __func__);

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	if (!ret) {
		if (error && *error) {
			g_warning ("[%s][%d][%s]",
				   g_quark_to_string ( (*error)->domain),
				   (*error)->code,
				   (*error)->message);
		}
		g_warning ("DBus dbus_g_proxy_call failed");
		goto cleanup;
	}

	*uri = _server_uri;

	g_debug ("%s called successfully", __func__);

cleanup:

	if (!ret) { // failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		if (error) {
			g_warning (" Error: %s", (*error)->message);
		}
	}

	g_debug ("%s--", __func__);
	return ret;
}


gboolean
eas_mail_handler_accept_provision_list (EasEmailHandler *self,
					const gchar* tid,
					const gchar* tid_status,
					GCancellable *cancellable,
					GError **error)
{
	EasEmailHandlerPrivate *priv = EAS_EMAIL_HANDLER_PRIVATE (self);
	gboolean ret = TRUE;
	DBusGProxy *proxy = priv->remoteCommonEas;
	guint cancel_handler_id;
	guint request_id = priv->next_request_id++;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (priv->account_uid ? priv->account_uid : "NULL"));

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_assert (self);

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_common_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call DBus API
	ret = dbus_g_proxy_call (proxy, "accept_provision_list",
				 error,
				 G_TYPE_STRING, priv->account_uid,
				 G_TYPE_STRING, tid,
				 G_TYPE_STRING, tid_status,
				 G_TYPE_INVALID,
				 G_TYPE_INVALID);

	g_debug ("%s - dbus proxy called", __func__);

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	if (!ret) {
		if (error && *error) {
			g_warning ("[%s][%d][%s]",
				   g_quark_to_string ( (*error)->domain),
				   (*error)->code,
				   (*error)->message);
		}
		g_warning ("DBus dbus_g_proxy_call failed");
		goto cleanup;
	}

	g_debug ("%s called successfully", __func__);

cleanup:

	if (!ret) { // failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		if (error) {
			g_warning (" Error: %s", (*error)->message);
		}
	}

	g_debug ("%s--", __func__);
	return ret;
}


/* sync emails in a specified folder (no bodies retrieved).
Returns lists of EasEmailInfos.
Max changes in one sync = 100 (configurable via a config api)
In the case of created emails all fields are filled in.
In the case of deleted emails only the serverids are valid.
In the case of updated emails only the serverids, flags and categories are valid.
*/
gboolean
eas_mail_handler_sync_folder_email_info (EasEmailHandler* self,
					 gchar *sync_key,
					 const gchar *collection_id, // folder to sync
					 GSList **emailinfos_created,
					 GSList **emailinfos_updated,
					 GSList **emailinfos_deleted,
					 gboolean *more_available,   // if there are more changes to sync (window_size exceeded)
					 GCancellable *cancellable,
					 GError **error)
{
	gboolean ret = FALSE;
	gchar **created_emailinfo_array = NULL;
	gchar **deleted_emailinfo_array = NULL;
	gchar **updated_emailinfo_array = NULL;
	gchar *updatedSyncKey = NULL;

	g_debug ("eas_mail_handler_sync_folder_email_info++");

	if (self == NULL || sync_key == NULL || collection_id == NULL || more_available == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_sync_folder_email_info requires valid arguments");
		goto out;
	}

	g_debug ("sync_key = %s", sync_key);

	ret = eas_gdbus_mail_call (self, "sync_folder_email",
				   NULL, NULL,
				   "(sss)", "(sb^as^as^as)",
				   NULL, error,
				   self->priv->account_uid, sync_key, collection_id,
				   &updatedSyncKey, more_available,
				   &created_emailinfo_array,
				   &deleted_emailinfo_array,
				   &updated_emailinfo_array);

	// convert created/deleted/updated emailinfo arrays into lists of emailinfo objects (deserialise results)
	if (ret) {
		g_debug ("sync_folder_email called successfully");

		// put the updated sync key back into the original string for tracking this
		/* When we get a null response (only headers because there were no changes),
		   do *NOT* overwrite the existing sync key! */
		if (updatedSyncKey && updatedSyncKey[0])
			strcpy (sync_key, updatedSyncKey);

		// get 3 arrays of strings of 'serialised' EasEmailInfos, convert to EasEmailInfo lists:
		ret = build_emailinfo_list ( (const gchar **) created_emailinfo_array, emailinfos_created, error);
		if (ret) {
			ret = build_emailinfo_list ( (const gchar **) deleted_emailinfo_array, emailinfos_deleted, error);
		}
		if (ret) {
			ret = build_emailinfo_list ( (const gchar **) updated_emailinfo_array, emailinfos_updated, error);
		}
	}

	if (updatedSyncKey) {
		g_free (updatedSyncKey);
	}
	free_string_array (created_emailinfo_array);
	free_string_array (updated_emailinfo_array);
	free_string_array (deleted_emailinfo_array);

	if (!ret) { // failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		g_slist_foreach (*emailinfos_created, (GFunc) g_object_unref, NULL);
		g_slist_free (*emailinfos_created);
		*emailinfos_created = NULL;
		g_slist_foreach (*emailinfos_updated, (GFunc) g_object_unref, NULL);
		g_slist_free (*emailinfos_updated);
		*emailinfos_updated = NULL;
		g_slist_foreach (*emailinfos_deleted, (GFunc) g_object_unref, NULL);
		g_slist_free (*emailinfos_deleted);
		*emailinfos_deleted = NULL;
	}
 out:
	g_debug ("eas_mail_handler_sync_folder_email_info--");
	g_debug ("sync_key = %s", sync_key);

	return ret;
}


static void
progress_signal_handler (DBusGProxy* proxy,
			 guint request_id,
			 guint percent,
			 gpointer user_data)
{
	EasProgressCallbackInfo *progress_callback_info;
	EasEmailHandler* self = (EasEmailHandler*) user_data;

	g_debug ("progress_signal_handler++");

	if ( (self->priv->email_progress_fns_table) && (percent > 0)) {
		// if there's a progress function for this request in our hashtable, call it:
		g_static_mutex_lock (&progress_table);
		progress_callback_info = g_hash_table_lookup (self->priv->email_progress_fns_table,
							      GUINT_TO_POINTER (request_id));
		g_static_mutex_unlock (&progress_table);
		if (progress_callback_info) {
			if (percent > progress_callback_info->percent_last_sent) {
				EasProgressFn progress_fn = (EasProgressFn) (progress_callback_info->progress_fn);

				g_debug ("call progress function with %d%c", percent, '%');
				progress_callback_info->percent_last_sent = percent;

				progress_fn (progress_callback_info->progress_data, percent);
			}
		}
	}

	g_debug ("progress_signal_handler--");
	return;
}


static void
dbus_call_completed (DBusGProxy* proxy, DBusGProxyCall* call, gpointer user_data)
{
	g_debug ("dbus call completed callback called");

	return;
}


// get the entire email body for listed email
// email body will be written to a file with the emailid as its name
gboolean
eas_mail_handler_fetch_email_body (EasEmailHandler* self,
				   const gchar *folder_id,
				   const gchar *server_id,
				   const gchar *mime_directory,
				   EasProgressFn progress_fn,
				   gpointer progress_data,
				   GCancellable *cancellable,
				   GError **error)
{
	gboolean ret = FALSE;

	g_debug ("eas_mail_handler_fetch_email_body++");

	if (self == NULL || folder_id == NULL || server_id == NULL || mime_directory == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_fetch_email_body requires valid arguments");
		goto out;
	}

	ret = eas_gdbus_mail_call (self, "fetch_email_body",
				   progress_fn, progress_data,
				   "(ssssu)", NULL,
				   cancellable, error,
				   self->priv->account_uid, folder_id,
				   server_id, mime_directory, 0);
out:
	g_debug ("eas_mail_handler_fetch_email_body--");
	return ret;
}


gboolean
eas_mail_handler_fetch_email_attachment (EasEmailHandler* self,
					 const gchar *file_reference,
					 const gchar *mime_directory,
					 EasProgressFn progress_fn,
					 gpointer progress_data,
					 GError **error)
{
	gboolean ret = FALSE;

	g_debug ("eas_mail_handler_fetch_email_attachment++");

	if (self == NULL || file_reference == NULL || mime_directory == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_fetch_email_attachment requires valid arguments");
		goto out;
	}

	ret = eas_gdbus_mail_call (self, "fetch_attachment",
				   progress_fn, progress_data,
				   "(sssu)", NULL,
				   NULL, error,
				   self->priv->account_uid, file_reference,
				   mime_directory, 0);

out:
	g_debug ("eas_mail_handler_fetch_email_attachments--");

	return ret;
}


// Delete specified emails from a single folder
gboolean
eas_mail_handler_delete_email (EasEmailHandler* self,
			       gchar *sync_key,            // sync_key for the folder containing these emails
			       const gchar *folder_id,     // folder that contains email to delete
			       const GSList *items_deleted,        // emails to delete
			       GCancellable *cancellable,
			       GError **error)
{
	gboolean ret = FALSE;
	gchar *ret_sync_key = NULL;
	gchar **deleted_items_array = NULL;
	// Build string array from items_deleted GSList
	guint list_length = g_slist_length ( (GSList*) items_deleted);
	int loop = 0;

	g_debug ("eas_mail_handler_delete_emails++");

	if (self == NULL || sync_key == NULL || items_deleted == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_sync_move_to_folder requires valid arguments");
		goto finish;
	}

	deleted_items_array = g_malloc0 ( (list_length + 1) * sizeof (gchar*));
	if (!deleted_items_array) {
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}

	for (; loop < list_length; ++loop) {
		deleted_items_array[loop] = g_strdup (g_slist_nth_data ( (GSList*) items_deleted, loop));
		g_debug ("Deleted Id: [%s]", deleted_items_array[loop]);
	}

	ret = eas_gdbus_mail_call (self, "delete_email",
				   NULL, NULL,
				   "(sss^as)", "(s)",
				   NULL, error,
				   self->priv->account_uid,
				   sync_key, folder_id,
				   deleted_items_array,
				   &ret_sync_key);

	// Clean up string array
	for (loop = 0; loop < list_length; ++loop) {
		g_free (deleted_items_array[loop]);
		deleted_items_array[loop] = NULL;
	}
	g_free (deleted_items_array);
	deleted_items_array = NULL;

finish:
	if (ret) {
		// put the updated sync key back into the original string for tracking this
		if (ret && ret_sync_key && ret_sync_key[0]) {
			strcpy (sync_key, ret_sync_key);
			g_debug ("%s ret_Sync_key %s", __func__, ret_sync_key);
		}

	} else {
		g_assert (error == NULL || *error != NULL);
	}

	if (ret_sync_key) {
		g_free (ret_sync_key);
	}

	g_debug ("eas_mail_handler_delete_emails--");
	return ret;
}


/*
'push' email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
TODO - should this be changed to support updating multiple emails at once?
*/
gboolean
eas_mail_handler_update_email (EasEmailHandler* self,
			       gchar *sync_key,            // sync_key for the folder containing the emails
			       const gchar *folder_id,     // folder that contains email to delete
			       GSList *update_emails,        // emails to update
			       GCancellable *cancellable,
			       GError **error)
{
	EasEmailHandlerPrivate *priv = self->priv;
	gboolean ret = TRUE;
	DBusGProxy *proxy = priv->remoteEas;
	// serialise the emails
	guint num_emails = g_slist_length ( (GSList *) update_emails);
	gchar **serialised_email_array = g_malloc0 ( (num_emails + 1) * sizeof (gchar*)); // null terminated array of strings
	gchar *serialised_email = NULL;
	gchar *ret_sync_key = NULL;
	gchar **ret_failed_updates_array = NULL;
	guint i = 0;
	GSList *l = (GSList *) update_emails;
	guint request_id = priv->next_request_id++;
	guint cancel_handler_id;

	g_debug ("eas_mail_handler_update_emails++");
	g_assert (self);
	g_assert (sync_key);
	g_assert (update_emails);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_debug ("sync_key = %s", sync_key);
	g_debug ("%d emails to update", num_emails);

	for (i = 0; i < num_emails; i++) {
		EasEmailInfo *email = l->data;

		g_debug ("serialising email %d", i);
		ret = eas_email_info_serialise (email, &serialised_email);
		if (!ret) {
			// set the error
			g_set_error (error, EAS_MAIL_ERROR,
				     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			serialised_email_array[i] = NULL;
			goto cleanup;
		}
		serialised_email_array[i] = serialised_email;
		g_debug ("serialised_email_array[%d] = %s", i, serialised_email_array[i]);

		l = l->next;
	}
	serialised_email_array[i] = NULL;

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_mail_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call dbus api
	ret = dbus_g_proxy_call (proxy, "update_emails", error,
				 G_TYPE_STRING, priv->account_uid,
				 G_TYPE_STRING, sync_key,
				 G_TYPE_STRING, folder_id,
				 G_TYPE_STRV, serialised_email_array,
				 G_TYPE_INVALID,
				 G_TYPE_STRING, &ret_sync_key,
				 G_TYPE_STRV, &ret_failed_updates_array,
				 G_TYPE_INVALID);

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	if (ret && ret_sync_key && ret_sync_key[0]) {
		g_debug ("%s ret_Sync_key %s", __func__, ret_sync_key);
		strcpy (sync_key, ret_sync_key);
	}

	// update status codes in update_emails where necessary

	//nb: looks like eas only provides a response when the status != ok
	if (!ret_failed_updates_array)
		goto cleanup;

	for (i = 0; ret_failed_updates_array[i] != NULL; i++) {
		// get the update response
		EasEmailInfo *email_failed = eas_email_info_new();

		g_debug ("ret_failed_updates_array[%d] = %s", i, ret_failed_updates_array[i]);

		eas_email_info_deserialise (email_failed, ret_failed_updates_array[i]);

		if (email_failed->status && strlen (email_failed->status)) {
			int idx;
			g_debug ("got status %s for update of %s", email_failed->status, email_failed->server_id);
			l = (GSList *) update_emails;

			for (idx = 0; idx < num_emails; idx++) {
				EasEmailInfo *email = l->data;
				if (strcmp (email_failed->server_id, email->server_id) == 0) {
					email->status = g_strdup (email_failed->status);
					break;	//out of inner for loop
				}
				l = l->next;
			}
		}
		g_free (ret_failed_updates_array[i]);
	}

	g_free (ret_failed_updates_array);
cleanup:
	// free all strings in the array
	for (i = 0; i < num_emails && serialised_email_array[i]; i++) {
		g_free (serialised_email_array[i]);
	}
	g_free (serialised_email_array);
	g_free (ret_sync_key);
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}

	g_debug ("eas_mail_handler_update_emails--");

	return ret;
}


gboolean
eas_mail_handler_send_email (EasEmailHandler* self,
			     const gchar *client_email_id,   // unique message identifier supplied by client
			     const gchar *mime_file,         // the full path to the email (mime) to be sent
			     EasProgressFn progress_fn,
			     gpointer progress_data,
			     GCancellable *cancellable,
			     GError **error)
{
	gboolean ret = FALSE;

	g_debug ("eas_mail_handler_send_email++");

	if (self == NULL || client_email_id == NULL || mime_file == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_send_email requires valid arguments");
		goto finish;
	}
	if (g_access (mime_file, R_OK)) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "Cannot read MIME file %s", mime_file);
		goto finish;
	}

	ret = eas_gdbus_mail_call (self, "send_email",
				   progress_fn, progress_data,
				   "(sssu)", NULL,
				   cancellable, error,
				   self->priv->account_uid,
				   client_email_id, mime_file, 0);

finish:
	g_debug ("eas_mail_handler_send_email--");

	return ret;
}

gboolean
eas_mail_handler_move_to_folder (EasEmailHandler* self,
				 const GSList *server_ids,
				 const gchar *src_folder_id,
				 const gchar *dest_folder_id,
				 GSList **updated_ids_list,
				 GError **error)
{
	gboolean ret = FALSE;
	gchar **updated_ids_array = NULL;
	gchar **server_ids_array = NULL;
	guint i = 0;
	guint list_len = g_slist_length ( (GSList*) server_ids);

	g_debug ("eas_mail_handler_move_to_folder++");

	if (self == NULL || server_ids == NULL || src_folder_id == NULL || dest_folder_id == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_sync_move_to_folder requires valid arguments");
		goto finish;
	}

	// convert lists to array for passing over dbus
	server_ids_array = g_malloc0 ( (list_len + 1) * sizeof (gchar*));
	if (!server_ids_array) {
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		goto finish;
	}

	for (; i < list_len; ++i) {
		server_ids_array[i] = g_strdup (g_slist_nth_data ( (GSList*) server_ids, i));
		g_debug ("server Id: [%s]", server_ids_array[i]);
	}

	ret = eas_gdbus_mail_call (self, "move_emails_to_folder",
				   NULL, NULL,
				   "(s^asss)", "(^as)",
				   NULL, error,
				   self->priv->account_uid, server_ids_array,
				   src_folder_id, dest_folder_id,
				   &updated_ids_array);

	// Clean up string array
	for (i = 0; i < list_len; ++i) {
		g_free (server_ids_array[i]);
		server_ids_array[i] = NULL;
	}
	g_free (server_ids_array);
	server_ids_array = NULL;

	if (ret) {
		ret = build_easidupdates_list ( (const gchar**) updated_ids_array, updated_ids_list, error);	//why does this require a cast?
	}
	g_strfreev (updated_ids_array);

finish:
	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_mail_handler_move_to_folder--");
	return ret;
}

// TODO How supported in AS?
gboolean
eas_mail_handler_copy_to_folder (EasEmailHandler* self,
				 const GSList *email_ids,
				 const gchar *src_folder_id,
				 const gchar *dest_folder_id,
				 GError **error)
{
	gboolean ret = TRUE;
	g_debug ("eas_mail_handler_copy_to_folder++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	g_debug ("eas_mail_handler_copy_to_folder--");

	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	return ret;
	/* TODO */
}

static void watch_email_folder_completed (DBusGProxy* proxy, DBusGProxyCall* call, gpointer user_data)
{
	GError *error = NULL;
	gchar* id;
	gchar **changed_folder_array = NULL;
	gint index = 0;
	GSList *folder_list = NULL;
	EasPushEmailCallback callback = NULL;

	g_debug ("watch_email_folder_completed");

	dbus_g_proxy_end_call (proxy, call, &error,
			       G_TYPE_STRV, &changed_folder_array,
			       G_TYPE_INVALID);

	while ( (id = changed_folder_array[index++])) {
		g_debug ("Folder id = [%s]", id);
		folder_list = g_slist_prepend (folder_list, g_strdup (id));
	}

	callback = (EasPushEmailCallback) (user_data);
	callback (folder_list, error);

	return;
}

gboolean eas_mail_handler_watch_email_folders (EasEmailHandler* self,
					       const GSList *folder_ids,
					       const gchar *heartbeat,
					       EasPushEmailCallback cb,
					       GError **error)
{
	gboolean ret = TRUE;
	DBusGProxy *proxy = self->priv->remoteEas;
	gchar **folder_array = NULL;
	// Build string array from items_deleted GSList
	guint list_length = g_slist_length ( (GSList*) folder_ids);
	int loop = 0;

	g_debug ("eas_mail_handler_watch_email_folders++");

	g_assert (self);
	g_assert (folder_ids);
	g_assert (heartbeat);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	folder_array = g_malloc0 ( (list_length + 1) * sizeof (gchar*));
	if (!folder_array) {
		g_set_error (error, EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
	}

	for (; loop < list_length; ++loop) {
		g_debug ("eas_mail_watch 1");
		folder_array[loop] = g_strdup (g_slist_nth_data ( (GSList*) folder_ids, loop));
		g_debug ("Folder Id: [%s]", folder_array[loop]);
	}


	g_debug ("eas_mail_handler_watch_email_folders1");
	// call dbus api
	dbus_g_proxy_begin_call (proxy, "watch_email_folders",
				 watch_email_folder_completed,
				 cb, 				//callback pointer
				 NULL, 						// destroy notification
				 G_TYPE_STRING, self->priv->account_uid,
				 G_TYPE_STRING, heartbeat,
				 G_TYPE_STRV, folder_array,
				 G_TYPE_INVALID);
	g_strfreev (folder_array);



	if (!ret) {
		g_assert (error == NULL || *error != NULL);
	}
	g_debug ("eas_mail_handler_watch_email_folders--");
	return ret;

}
gboolean
eas_mail_handler_sync_folder_email (EasEmailHandler* self,
				    const gchar *sync_key_in,
				    guint	time_window,
				    const gchar *folder_id,
				    GSList *delete_emails,	// list of server ids
				    GSList *change_emails, 	// list of EasMailInfos
				    gchar **sync_key_out,
				    GSList **emails_created,
				    GSList **emails_changed,
				    GSList **emails_deleted,
				    gboolean *more_available,
				    EasProgressFn progress_fn,
				    gpointer progress_data,
				    GCancellable *cancellable,
				    GError **error)
{
	gboolean ret = TRUE;
	EasEmailHandlerPrivate *priv = self->priv;
	DBusGProxy *proxy_common = priv->remoteCommonEas;	// uses the common object, not the email object
	gchar **created_emailinfo_array = NULL;
	gchar **deleted_emailinfo_array = NULL;
	gchar **changed_emailinfo_array = NULL;
	guint request_id;
	gchar **delete_emails_array = NULL;
	gchar **ret_failed_changes_array = NULL;
	guint delete_list_length = g_slist_length ( (GSList*) delete_emails);
	guint change_list_length = g_slist_length ( (GSList *) change_emails);
	gchar **change_emails_array = g_malloc0 ( (change_list_length + 1) * sizeof (gchar *));   // null terminated
	gchar *serialised_email = NULL;
	int loop = 0;
	guint i = 0;
	GSList *l = (GSList *) change_emails;
	DBusGProxyCall *call;
	guint cancel_handler_id;

	g_debug ("eas_mail_handler_sync_folder_email++");

	g_debug ("delete_list_length = %d", delete_list_length);
	g_debug ("change_list_length = %d", change_list_length);

	if ( (!self) || (!sync_key_in) || (!folder_id) || (!sync_key_out) || (!more_available)) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "bad argument passed to eas_mail_handler_sync_folder_email");
		ret = FALSE;
		goto cleanup;
	}

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	// if there's a progress function supplied, add it (and the progress_data) to the hashtable, indexed by id
	request_id = priv->next_request_id++;

	if (progress_fn) {
		ret = eas_mail_add_progress_info_to_table (self, request_id, progress_fn, progress_data, error);
		if (!ret)
			goto finish;
	}

	// build string array from delete_emails list
	for (; loop < delete_list_length; ++loop) {
		delete_emails_array[loop] = g_strdup (g_slist_nth_data ( (GSList*) delete_emails, loop));
		g_debug ("Deleted Id: [%s]", delete_emails_array[loop]);
	}

	// build string array from update_emails list
	for (i = 0; i < change_list_length; i++) {
		EasEmailInfo *email = l->data;

		g_debug ("serialising email %d", i);
		ret = eas_email_info_serialise (email, &serialised_email);
		if (!ret) {
			// set the error
			g_set_error (error, EAS_MAIL_ERROR,
				     EAS_MAIL_ERROR_NOTENOUGHMEMORY,
				     ("out of memory"));
			change_emails_array[i] = NULL;
			goto finish;
		}
		change_emails_array[i] = serialised_email;
		g_debug ("change_emails_array[%d] = %s", i, change_emails_array[i]);

		l = l->next;
	}
	change_emails_array[i] = NULL;

	if (cancellable) {
		EasCancelInfo *cancel_info = g_new0 (EasCancelInfo, 1);	// freed on disconnect

		cancel_info->handler = self;
		cancel_info->request_id = request_id;
		// connect to the "cancelled" signal
		g_debug ("connect to cancellable");
		cancel_handler_id = g_cancellable_connect (cancellable,
							   G_CALLBACK (eas_mail_handler_cancel_common_request),
							   (gpointer) cancel_info,
							   g_free);				// data destroy func
	}

	// call dbus api with appropriate params
	call = dbus_g_proxy_begin_call (proxy_common,
					"sync_folder_items",
					dbus_call_completed,
					NULL, 							// userdata passed to callback
					NULL, 							// destroy notification
					G_TYPE_STRING, self->priv->account_uid,
					G_TYPE_UINT, EAS_ITEM_MAIL,
					G_TYPE_STRING, sync_key_in,
					G_TYPE_STRING, folder_id,
					G_TYPE_UINT, time_window,
					G_TYPE_STRV, NULL,						// add items - always NULL for email
					G_TYPE_STRV, delete_emails_array,      // delete items.
					G_TYPE_STRV, change_emails_array,		// change items
					G_TYPE_UINT, request_id,
					G_TYPE_INVALID);

	g_debug ("block until results available");

	// blocks until results are available:
	ret = dbus_g_proxy_end_call (proxy_common,
				     call,
				     error,
				     G_TYPE_STRING, sync_key_out,
				     G_TYPE_BOOLEAN, more_available,
				     G_TYPE_STRV, &created_emailinfo_array,
				     G_TYPE_STRV, &deleted_emailinfo_array,
				     G_TYPE_STRV, &changed_emailinfo_array,
				     G_TYPE_STRV, NULL,
				     G_TYPE_STRV, NULL,
				     G_TYPE_STRV, &ret_failed_changes_array,
				     G_TYPE_INVALID);

	g_debug ("dbus call returned");

	if (cancellable) {
		// disconnect from cancellable
		g_debug ("disconnect from cancellable");
		g_cancellable_disconnect (cancellable, cancel_handler_id);
	}

	// convert created/deleted/changed emailinfo arrays into lists of emailinfo objects (deserialise results)
	if (ret) {
		g_debug ("sync_folder_email called successfully");

		// get 3 arrays of 'serialised' EasEmailInfos, convert to EasEmailInfo lists:
		ret = build_emailinfo_list ( (const gchar **) created_emailinfo_array, emails_created, error);
		if (ret) {
			ret = build_emailinfo_list ( (const gchar **) deleted_emailinfo_array, emails_deleted, error);
		}
		if (ret) {
			ret = build_emailinfo_list ( (const gchar **) changed_emailinfo_array, emails_changed, error);
		}

		// update status codes in update_emails where necessary

		for (i = 0; ret_failed_changes_array[i] != NULL; i++) {
			// get the update response
			EasEmailInfo *email_failed = eas_email_info_new();

			g_debug ("ret_failed_changes_array[%d] = %s", i, ret_failed_changes_array[i]);

			eas_email_info_deserialise (email_failed, ret_failed_changes_array[i]);

			if (email_failed->status && strlen (email_failed->status)) {
				int idx;
				g_debug ("got status %s for update of %s", email_failed->status, email_failed->server_id);
				l = (GSList *) change_emails;

				for (idx = 0; idx < change_list_length; idx++) {
					EasEmailInfo *email = l->data;
					if (strcmp (email_failed->server_id, email->server_id) == 0) {
						email->status = g_strdup (email_failed->status);
						break;	//out of inner for loop
					}
					l = l->next;
				}
			}
		}

		free_string_array (created_emailinfo_array);
		free_string_array (changed_emailinfo_array);
		free_string_array (deleted_emailinfo_array);

		// free the delete_emails array
		for (loop = 0; loop < delete_list_length; ++loop) {
			g_free (delete_emails_array[loop]);
			delete_emails_array[loop] = NULL;
		}
		g_free (delete_emails_array);
		delete_emails_array = NULL;
		// free ret_failed_updates_array
		for (i = 0; ret_failed_changes_array[i] != NULL; i++) {
			g_free (ret_failed_changes_array[i]);
		}
		g_free (ret_failed_changes_array);
	}

finish:
	g_hash_table_remove (priv->email_progress_fns_table,
			     GUINT_TO_POINTER (request_id));
cleanup:
	// free the change_emails array
	for (i = 0; i < change_list_length && change_emails_array[i]; i++) {
		g_free (change_emails_array[i]);
	}
	g_free (change_emails_array);
	if (!ret) {
		// failed - cleanup lists
		g_assert (error == NULL || *error != NULL);
		if (*emails_created) {
			g_slist_foreach (*emails_created, (GFunc) g_object_unref, NULL);
			g_slist_free (*emails_created);
			*emails_created = NULL;
		}
		if (*emails_changed) {
			g_slist_foreach (*emails_changed, (GFunc) g_object_unref, NULL);
			g_slist_free (*emails_changed);
			*emails_changed = NULL;
		}
		if (*emails_deleted) {
			g_slist_foreach (*emails_deleted, (GFunc) g_object_unref, NULL);
			g_slist_free (*emails_deleted);
			*emails_deleted = NULL;
		}
	}
	g_debug ("eas_mail_handler_sync_folder_email_info--");

	return ret;
}

static void
eas_mail_handler_cancel_request (GCancellable *cancellable, gpointer user_data, const gchar *path, const gchar *iface)
{
	EasCancelInfo *cancel_info = user_data;
	EasEmailHandler* self = cancel_info->handler;
	EasEmailHandlerPrivate *priv = self->priv;
	gboolean ret;
	guint request_id = cancel_info->request_id;
	GError *error = NULL;
	DBusGConnection *bus;
	DBusGProxy *proxy;

	g_debug ("eas_mail_handler_cancel_request++");

	g_debug ("call dbus to cancel request with id %d", request_id);

	// create (and the tear down) a new dbus session to connect to dbus interface
	// to allow us to send the cancel to dbus even if email handler's main
	// dbus session is blocked
	g_debug ("Connecting to new D-Bus Session to cancel");
	bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (bus == NULL) {
		g_warning ("Error: Couldn't connect to the Session bus (%s) ", error->message);
	}

	g_debug ("Creating a GLib proxy object for Eas.");
	proxy =  dbus_g_proxy_new_for_name (bus,
					    EAS_SERVICE_NAME,
					    path,
					    iface);

	if (proxy == NULL) {
		g_warning ("Error: Couldn't create the proxy object");
	}

	// call the cancel operation on the common interface
	ret = dbus_g_proxy_call (proxy,
				 "cancel_request",
				 &error,
				 G_TYPE_STRING, priv->account_uid,
				 G_TYPE_UINT, request_id,
				 G_TYPE_INVALID,
				 G_TYPE_INVALID);	// no out params. fire and forget

	g_debug ("dbus call to cancel request returned");

	if (!ret) {
		g_warning ("cancel request failed with %s", error->message);
		g_error_free (error);
	}

	// disconnect from dbus
	dbus_g_connection_unref (bus);

	g_object_unref (proxy);

	g_debug ("eas_mail_handler_cancel_common_request--");
}

static void
eas_mail_handler_cancel_common_request (GCancellable *cancellable, gpointer user_data)
{
	g_debug ("eas_mail_handler_cancel_common_request++");
	eas_mail_handler_cancel_request	(cancellable, user_data, EAS_SERVICE_COMMON_OBJECT_PATH, EAS_SERVICE_COMMON_INTERFACE);
	g_debug ("eas_mail_handler_cancel_common_request--");

}

static void
eas_mail_handler_cancel_mail_request (GCancellable *cancellable, gpointer user_data)
{
	g_debug ("eas_mail_handler_cancel_mail_request++");
	eas_mail_handler_cancel_request	(cancellable, user_data, EAS_SERVICE_MAIL_OBJECT_PATH, EAS_SERVICE_MAIL_INTERFACE);
	g_debug ("eas_mail_handler_cancel_mail_request--");
}
