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

#include "eas-dbus-client.h"
#include "eas-errors.h"
#include <string.h>

gboolean
eas_gdbus_client_init (struct eas_gdbus_client *client, const gchar *account_uid, GError **error)
{
	client->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, error);
	if (!client->connection)
		return FALSE;

	client->account_uid = g_strdup (account_uid);
#if GLIB_CHECK_VERSION (2,31,0)
	client->progress_lock = &client->_mutex;
	g_mutex_init (client->progress_lock);
	client->progress_cond = &client->_cond;
	g_cond_init (client->progress_cond);
#else
	client->progress_lock = g_mutex_new ();
	client->progress_cond = g_cond_new ();
#endif
	client->progress_fns_table = g_hash_table_new_full (NULL, NULL, NULL, g_free);
	return TRUE;
}

void
eas_gdbus_client_destroy (struct eas_gdbus_client *client)
{
	if (client->connection) {
		g_object_unref (client->connection);
		client->connection = NULL;
	}

	if (client->progress_lock) {
#if GLIB_CHECK_VERSION (2,31,0)
		g_mutex_clear (client->progress_lock);
#else
		g_mutex_free (client->progress_lock);
#endif
		client->progress_lock = NULL;
	}

	if (client->progress_cond) {
#if GLIB_CHECK_VERSION (2,31,0)
		g_cond_clear (client->progress_cond);
#else
		g_cond_free (client->progress_cond);
#endif
		client->progress_cond = NULL;
	}

	g_free (client->account_uid);
	client->account_uid = NULL;

	if (client->progress_fns_table) {
		g_hash_table_remove_all (client->progress_fns_table);
		g_hash_table_unref (client->progress_fns_table);
		client->progress_fns_table = NULL;
	}
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

gboolean
eas_gdbus_call_finish (struct eas_gdbus_client *client, GAsyncResult *result,
		       guint cancel_serial, const gchar *out_params,
		       va_list *ap, GError **error)
{
	GDBusMessage *reply;
	gchar *out_params_type = (gchar *) out_params;
	gboolean success = FALSE;
	GVariant *v;

	reply = g_dbus_connection_send_message_with_reply_finish(client->connection,
								 result, error);
	if (cancel_serial) {
		GDBusMessage *message;

		message = g_dbus_message_new_method_call (EAS_SERVICE_NAME,
							  EAS_SERVICE_COMMON_OBJECT_PATH,
							  EAS_SERVICE_COMMON_INTERFACE,
							  "cancel_request");
		g_dbus_message_set_body (message,
					 g_variant_new ("(su)",
							client->account_uid,
							cancel_serial));

		g_dbus_connection_send_message (client->connection,
						message,
						G_DBUS_SEND_MESSAGE_FLAGS_NONE,
						NULL, NULL);

		g_object_unref (message);
	}

	if (!reply)
		return FALSE;

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
		g_variant_get_va (v, out_params, NULL, ap);
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
	return success;
}

struct _EasProgressCallbackInfo {
	EasProgressFn progress_fn;
	gpointer progress_data;
	guint percent_last_sent;
	gboolean calling;

	guint handler_id;
	GCancellable cancellable;
};

typedef struct _EasProgressCallbackInfo EasProgressCallbackInfo;

static gboolean
eas_client_add_progress_info_to_table (struct eas_gdbus_client *client, guint request_id, EasProgressFn progress_fn, gpointer progress_data, GError **error)
{
	gboolean ret = TRUE;
	EasProgressCallbackInfo *progress_info = g_malloc0 (sizeof (EasProgressCallbackInfo));

	if (!progress_info) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));
		ret = FALSE;
		goto finish;
	}

	// add progress fn/data structure to hash table
	progress_info->progress_fn = progress_fn;
	progress_info->progress_data = progress_data;
	progress_info->percent_last_sent = 0;

	g_mutex_lock (client->progress_lock);
	g_debug ("insert progress function into table");
	g_hash_table_insert (client->progress_fns_table, GUINT_TO_POINTER (request_id), progress_info);
	g_mutex_unlock (client->progress_lock);
finish:
	return ret;
}

gboolean
eas_gdbus_call (struct eas_gdbus_client *client, const gchar *object,
		const gchar *interface, const gchar *method,
		EasProgressFn progress_fn, gpointer progress_data,
		const gchar *in_params, const gchar *out_params,
		GCancellable *cancellable, GError **error, ...)
{
	GDBusMessage *message;
	struct _eas_call_data call;
	GMainContext *ctxt;
	GVariant *v = NULL;
	va_list ap;
	gboolean success;
	guint cancel_handler_id = 0;
	guint32 serial = 0;

	va_start (ap, error);

	message = g_dbus_message_new_method_call (EAS_SERVICE_NAME, object,
						  interface, method);

	v = g_variant_new_va (in_params, NULL, &ap);
	g_dbus_message_set_body (message, v);

	call.cancelled = FALSE;
	call.result = NULL;
	ctxt = g_main_context_new ();
	call.loop = g_main_loop_new (ctxt, FALSE);

	g_main_context_push_thread_default (ctxt);

	g_dbus_connection_send_message_with_reply (client->connection,
						   message,
						   G_DBUS_SEND_MESSAGE_FLAGS_NONE,
						   1000000,
						   &serial,
						   cancellable,
						   _call_done,
						   (gpointer) &call);
	g_object_unref (message);

	if (cancellable)
		cancel_handler_id = g_cancellable_connect (cancellable,
							  G_CALLBACK (_call_cancel),
							  (gpointer) &call, NULL);

	/* Ignore error; it's not the end of the world if progress info
	   is lost, and it should never happen anyway */
	if (progress_fn)
		eas_client_add_progress_info_to_table (client, serial, progress_fn,
						       progress_data, NULL);

	g_main_loop_run (call.loop);

	if (cancellable)
		g_cancellable_disconnect (cancellable, cancel_handler_id);

	success = eas_gdbus_call_finish (client, call.result, call.cancelled ? serial : 0,
					 out_params, &ap, error);

	if (serial && progress_fn) {
		EasProgressCallbackInfo *cbinfo;

		g_mutex_lock (client->progress_lock);
		cbinfo = g_hash_table_lookup (client->progress_fns_table,
					      GUINT_TO_POINTER (serial));
		if (cbinfo && cbinfo->calling) {
			g_debug ("Progress for call %u is running; wait for it to complete",
				 serial);
			g_hash_table_steal (client->progress_fns_table,
					    GUINT_TO_POINTER (serial));
			do {
				g_cond_wait (client->progress_cond, client->progress_lock);
			} while (cbinfo->calling);
			g_free (cbinfo);
		} else if (cbinfo) {
			g_hash_table_remove (client->progress_fns_table,
					     GUINT_TO_POINTER (serial));
		}
		g_mutex_unlock (client->progress_lock);
	}

	va_end (ap);

	g_main_context_pop_thread_default (ctxt);
	g_main_context_unref (ctxt);
	g_main_loop_unref (call.loop);
	g_object_unref (call.result);

	return success;
}

static void
progress_signal_handler(GDBusConnection *connection,
			const gchar *sender_name,
			const gchar *object_path,
			const gchar *interface_name,
			const gchar *signal_name,
			GVariant *parameters,
			gpointer user_data)
{
	EasProgressCallbackInfo *progress_callback_info;
	struct eas_gdbus_client *client = user_data;
	guint request_id, percent;

	g_debug ("progress_signal_handler++");

	if (!parameters) {
		g_debug ("Got progress signal with no parameters\n");
		goto out;
	}

	if (!g_variant_is_of_type (parameters, G_VARIANT_TYPE ("(uu)"))) {
		g_debug ("Got progress signal with wrong parameters (%s)\n",
			 g_variant_get_type_string (parameters));
		goto out;
	}

	g_variant_get (parameters, "(uu)", &request_id, &percent);

	if (percent > 0) {
		// if there's a progress function for this request in our hashtable, call it:
		g_mutex_lock (client->progress_lock);
		progress_callback_info = g_hash_table_lookup (client->progress_fns_table,
							      GUINT_TO_POINTER (request_id));
		if (progress_callback_info &&
		    percent > progress_callback_info->percent_last_sent) {
			EasProgressFn progress_fn = (EasProgressFn) (progress_callback_info->progress_fn);

			progress_callback_info->calling = TRUE;
			g_mutex_unlock (client->progress_lock);

			g_debug ("call progress function with %d%c", percent, '%');
			progress_callback_info->percent_last_sent = percent;

			progress_fn (progress_callback_info->progress_data, percent);

			g_mutex_lock (client->progress_lock);
			progress_callback_info->calling = FALSE;
			g_cond_broadcast (client->progress_cond);
		}
		g_mutex_unlock (client->progress_lock);
	}

 out:
	g_debug ("progress_signal_handler--");
	return;
}

guint
eas_gdbus_progress_subscribe (struct eas_gdbus_client *client,
			      const gchar *interface,
			      const gchar *signal,
			      const gchar *object)
{
	return g_dbus_connection_signal_subscribe (client->connection,
						   EAS_SERVICE_NAME, interface,
						   signal, object, NULL,
						   G_DBUS_SIGNAL_FLAGS_NONE,
						   progress_signal_handler,
						   client, NULL);
}

void eas_gdbus_progress_unsubscribe (struct eas_gdbus_client *client,
				     guint handle)
{
	g_dbus_connection_signal_unsubscribe (client->connection, handle);
}
