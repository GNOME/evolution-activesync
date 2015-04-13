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
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlsave.h>

#include "eas-errors.h"
#include "../eas-daemon/src/activesyncd-common-defs.h"
#include "libeassync.h"
#include "libeasmail.h"
#include "eas-folder.h"
#include "eas-mail-errors.h"
#include "eas-logger.h"
#include "eas-dbus-client.h"

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
	
struct _EasEmailHandlerPrivate {
	struct eas_gdbus_client eas_client;

	guint mail_signal;
	guint common_signal;
};

// TODO - how much verification of args should happen??

static void
eas_mail_handler_init (EasEmailHandler *cnc)
{
	EasEmailHandlerPrivate *priv;
	g_debug ("eas_mail_handler_init++");

	/* allocate internal structure */
	cnc->priv = priv = EAS_EMAIL_HANDLER_PRIVATE (cnc);

	memset (&priv->eas_client, 0, sizeof (priv->eas_client));
	g_debug ("eas_mail_handler_init--");
}

static void
eas_mail_handler_dispose (GObject *object)
{
#if 0
	EasEmailHandler *cnc = EAS_EMAIL_HANDLER (object);
	EasEmailHandlerPrivate *priv;
	priv = cnc->priv;
#endif
	G_OBJECT_CLASS (eas_mail_handler_parent_class)->dispose (object);
}


static void
eas_mail_handler_finalize (GObject *object)
{
	EasEmailHandler *cnc = EAS_EMAIL_HANDLER (object);
	EasEmailHandlerPrivate *priv;
	g_debug ("eas_mail_handler_finalize++");

	priv = cnc->priv;

	eas_gdbus_progress_unsubscribe (&priv->eas_client, priv->mail_signal);
	eas_gdbus_progress_unsubscribe (&priv->eas_client, priv->common_signal);

	eas_gdbus_client_destroy (&priv->eas_client);

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

EasEmailHandler *
eas_mail_handler_new (const char* account_uid, GError **error)
{
	EasEmailHandler *object = NULL;
	EasEmailHandlerPrivate *priv = NULL;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif

	/* Ick. See https://bugzilla.gnome.org/show_bug.cgi?id=662396 */
	eas_connection_error_quark();

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

	if (!eas_gdbus_client_init (&priv->eas_client, account_uid, error)) {
		g_object_unref (object);
		return NULL;
	}

	priv->mail_signal = eas_gdbus_progress_subscribe (&priv->eas_client,
							  EAS_SERVICE_MAIL_INTERFACE,
							  EAS_MAIL_SIGNAL_PROGRESS,
							  EAS_SERVICE_MAIL_OBJECT_PATH);

	priv->common_signal = eas_gdbus_progress_subscribe (&priv->eas_client,
							   EAS_SERVICE_COMMON_INTERFACE,
							   EAS_MAIL_SIGNAL_PROGRESS,
							   EAS_SERVICE_COMMON_OBJECT_PATH);
	g_debug ("eas_mail_handler_new--");
	return object;
}

#define eas_gdbus_mail_call(self, ...) eas_gdbus_call(&(self)->priv->eas_client, EAS_SERVICE_MAIL_OBJECT_PATH, EAS_SERVICE_MAIL_INTERFACE, __VA_ARGS__)
#define eas_gdbus_common_call(self, ...) eas_gdbus_call(&(self)->priv->eas_client, EAS_SERVICE_COMMON_OBJECT_PATH, EAS_SERVICE_COMMON_INTERFACE, __VA_ARGS__)

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

gboolean eas_mail_handler_get_item_estimate (EasEmailHandler* self,
					     const gchar *sync_key,
					     const gchar *folder_id, // folder to sync
					     guint *estimate,
					     GError **error)
{
	return eas_gdbus_mail_call (self, "get_item_estimate",
				    NULL, NULL, "(sss)", "(u)",
				    NULL, error,
				    self->priv->eas_client.account_uid, sync_key, folder_id,
				    estimate);
}

gboolean
eas_mail_handler_get_folder_list (EasEmailHandler *self,
				  gboolean force_refresh,
				  GSList **folders,
				  GCancellable *cancellable,
				  GError **error)
{
	gboolean ret = FALSE;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (self->priv->eas_client.account_uid ? self->priv->eas_client.account_uid : "NULL"));

	if (self == NULL || folders == NULL || *folders != NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "%s requires valid arguments", __func__);
		goto out;
	}

	ret = eas_folder_get_folder_list (&(self)->priv->eas_client,
					  force_refresh, folders, cancellable, error);

 out:
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
	gboolean ret = FALSE;
	gchar* _provision_list_buffer = NULL;
	gchar* _tid = NULL;
	gchar* _tid_status = NULL;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (self->priv->eas_client.account_uid ? self->priv->eas_client.account_uid : "NULL"));

	if (self == NULL || tid == NULL || tid_status == NULL || provision_list == NULL ||
	    *tid != NULL || *tid_status != NULL || *provision_list != NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR, EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_get_provision_list requires valid arguments");
		goto out;
	}

	ret = eas_gdbus_common_call (self, "get_provision_list", NULL, NULL,
				     "(s)", "(sss)", cancellable, error,
				     self->priv->eas_client.account_uid, &_tid, &_tid_status,
				     &_provision_list_buffer);

	g_debug ("%s - dbus proxy called", __func__);

	if (!ret)
		goto out;

	g_debug ("%s called successfully", __func__);

	*tid = _tid; // full transfer
	*tid_status = _tid_status; // full transfer

	// Build the provision list
	*provision_list = eas_provision_list_new();
	ret = eas_provision_list_deserialise (*provision_list, _provision_list_buffer);
	if (!ret) {
		g_set_error (error,
			     EAS_MAIL_ERROR, EAS_MAIL_ERROR_UNKNOWN,
			     "eas_mail_handler_get_provision_list failed to unmarshal arguments");
	}

	g_free (_provision_list_buffer);

	if (!ret && *provision_list) {
		g_debug ("%s failure - cleanup lists", __func__);
		g_object_unref (*provision_list);
		*provision_list = NULL;
	}
 out:
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
	gboolean ret = FALSE;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (self->priv->eas_client.account_uid ? self->priv->eas_client.account_uid : "NULL"));

	// call DBus API
	ret = eas_gdbus_common_call (self, "autodiscover", NULL, NULL,
				     "(ss)", "(s)", cancellable, error,
				     email, username, uri);

	g_debug ("%s - dbus proxy called %ssuccessfully", __func__,
		 ret ? "": "un");

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
	gboolean ret = FALSE;

	g_debug ("%s++ : account_uid[%s]", __func__,
		 (self->priv->eas_client.account_uid ? self->priv->eas_client.account_uid : "NULL"));

	// call DBus API
	ret = eas_gdbus_common_call (self, "accept_provision_list",
				     NULL, NULL, "(sss)", NULL,
				     cancellable, error,
				     self->priv->eas_client.account_uid,
				     tid, tid_status);

	g_debug ("%s - dbus proxy called %ssuccessfully", __func__,
		 ret ? "": "un");

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
				   self->priv->eas_client.account_uid, sync_key, collection_id,
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
	g_strfreev (created_emailinfo_array);
	g_strfreev (updated_emailinfo_array);
	g_strfreev (deleted_emailinfo_array);

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
				   self->priv->eas_client.account_uid, folder_id,
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
				   self->priv->eas_client.account_uid, file_reference,
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
				   self->priv->eas_client.account_uid,
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
	gboolean ret = FALSE;
	// serialise the emails
	guint num_emails = g_slist_length ( (GSList *) update_emails);
	gchar **serialised_email_array = g_malloc0 ( (num_emails + 1) * sizeof (gchar*)); // null terminated array of strings
	gchar *serialised_email = NULL;
	gchar *ret_sync_key = NULL;
	gchar **ret_failed_updates_array = NULL;
	guint i = 0;
	GSList *l = (GSList *) update_emails;

	g_debug ("eas_mail_handler_update_emails++");

	if (self == NULL || sync_key == NULL || update_emails == NULL) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "eas_mail_handler_update_email requires valid arguments");
		goto cleanup;
	}

	g_debug ("sync_key = %s", sync_key);
	g_debug ("%d emails to update", num_emails);

	for (i = 0; i < num_emails; i++) {
		EasEmailInfo *email = l->data;

		g_debug ("serialising email %d", i);
		ret = eas_email_info_serialise (email, &serialised_email);
		if (!ret) {
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

	ret = eas_gdbus_mail_call (self, "update_emails",
				   NULL, NULL,
				   "(sss^as)", "(s^as)",
				   cancellable, error,
				   self->priv->eas_client.account_uid,
				   sync_key, folder_id,
				   serialised_email_array,
				   &ret_sync_key,
				   &ret_failed_updates_array);

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
				   self->priv->eas_client.account_uid,
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
				   self->priv->eas_client.account_uid, server_ids_array,
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

struct _eas_ping_call {
	EasEmailHandler *handler;
	guint cancel_handler_id;
	guint32 serial;
	GCancellable *cancellable;
	gboolean cancelled;
	EasPushEmailCallback cb;
	gpointer cbdata;
};

/* Ick. eas_gdbus_call_finish maybe shouldn't take a va_list *, and then we wouldn't
   have to jump through this hoop. Maybe make it return a GVariant? */
static gboolean
eas_gdbus_ping_finish (struct eas_gdbus_client *client, GAsyncResult *result, guint cancel_serial,
		       const gchar *out_params, GError **error, ...)
{
	va_list ap;
	gboolean ret;

	va_start (ap, error);
	ret = eas_gdbus_call_finish (client, result, cancel_serial, out_params, &ap, error);
	va_end (ap);

	return ret;
}

static void
_ping_done (GObject *conn, GAsyncResult *result, gpointer _call)
{
	struct _eas_ping_call *call = _call;
	GDBusMessage *reply;
	GError *error = NULL;
	GSList *folder_list = NULL;
	gchar **results = NULL;
	int i;

	reply = g_dbus_connection_send_message_with_reply_finish (call->handler->priv->eas_client.connection,
								  result, &error);
	if (reply) {
		eas_gdbus_ping_finish (&call->handler->priv->eas_client, result,
				       call->cancelled ? call->serial : 0,
				       "(^as)", &error, &results);

		if (results) {
			for (i=0; results[i]; i++) {
				g_debug ("Folder '%s' changed in Ping results", results[i]);
				folder_list = g_slist_append (folder_list, g_strdup (results[i]));
			}
			g_strfreev (results);
		}
		g_object_unref (reply);
	}
	/* Callee must free list *and* error */
	call->cb (folder_list, error);

	g_object_unref (call->handler);
	g_clear_error (&error);
	g_slist_foreach (folder_list, (GFunc)g_free, NULL);
	g_slist_free (folder_list);

	if (call->cancellable) {
		g_cancellable_disconnect (call->cancellable, call->cancel_handler_id);
		g_object_unref (call->cancellable);
	}

	g_slice_free (struct _eas_ping_call, call);
}

static void
_ping_cancel (GCancellable *cancellable, gpointer _call)
{
	struct _eas_ping_call *call = _call;

	/* We can't send the DBus message from here; the GDBusConnection will
	   deadlock itself in g_cancellable_disconnect, because it wants its
	   *own* cancel handler to complete but ends up waiting for *this* one,
	   which in turn is waiting for the connection lock. */
	call->cancelled = TRUE;
}

gboolean eas_mail_handler_watch_email_folders (EasEmailHandler* self,
					       const GSList *folder_ids,
					       const gchar *heartbeat,
					       EasPushEmailCallback cb,
					       GError **error)
{
	GCancellable *cancellable = NULL;
	GDBusMessage *message;
	gchar **folder_array = NULL;
	guint list_length = g_slist_length ( (GSList*) folder_ids);
	struct _eas_ping_call *call;
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
		return FALSE;
	}

	for (; loop < list_length; ++loop) {
		g_debug ("eas_mail_watch 1");
		folder_array[loop] = g_strdup (g_slist_nth_data ( (GSList*) folder_ids, loop));
		g_debug ("Folder Id: [%s]", folder_array[loop]);
	}


	g_debug ("eas_mail_handler_watch_email_folders1");
	message = g_dbus_message_new_method_call (EAS_SERVICE_NAME,
						  EAS_SERVICE_MAIL_OBJECT_PATH,
						  EAS_SERVICE_MAIL_INTERFACE,
						  "watch_email_folders");
	g_dbus_message_set_body (message,
				 g_variant_new ("(ss^as)",
						self->priv->eas_client.account_uid,
						heartbeat, folder_array));
	g_strfreev (folder_array);

	call = g_slice_new0 (struct _eas_ping_call);
	call->handler = g_object_ref (self);
	call->cb = cb;
	if (cancellable) {
		call->cancellable = g_object_ref (cancellable);
		call->cancel_handler_id = g_cancellable_connect (cancellable,
								 G_CALLBACK (_ping_cancel),
								 (gpointer) call, NULL);
	}

	g_dbus_connection_send_message_with_reply (self->priv->eas_client.connection,
						   message,
						   G_DBUS_SEND_MESSAGE_FLAGS_NONE,
						   1000000,
						   &call->serial,
						   cancellable,
						   _ping_done,
						   call);
	g_object_unref (message);

	g_debug ("eas_mail_handler_watch_email_folders--");
	return TRUE;

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
	gboolean ret = FALSE;
	gchar **created_emailinfo_array = NULL;
	gchar **deleted_emailinfo_array = NULL;
	gchar **changed_emailinfo_array = NULL;
	gchar **delete_emails_array = NULL;
	gchar **ret_failed_changes_array = NULL;
	guint delete_list_length = g_slist_length ( (GSList*) delete_emails);
	guint change_list_length = g_slist_length ( (GSList *) change_emails);
	gchar **change_emails_array = g_malloc0 ( (change_list_length + 1) * sizeof (gchar *));   // null terminated
	gchar *serialised_email = NULL;
	int loop = 0;
	guint i = 0;
	GSList *l = (GSList *) change_emails;

	g_debug ("eas_mail_handler_sync_folder_email++");

	g_debug ("delete_list_length = %d", delete_list_length);
	g_debug ("change_list_length = %d", change_list_length);

	if ( (!self) || (!sync_key_in) || (!folder_id) || (!sync_key_out) || (!more_available)) {
		g_set_error (error,
			     EAS_MAIL_ERROR,
			     EAS_MAIL_ERROR_BADARG,
			     "bad argument passed to eas_mail_handler_sync_folder_email");
		goto cleanup;
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
			goto cleanup;
		}
		change_emails_array[i] = serialised_email;
		g_debug ("change_emails_array[%d] = %s", i, change_emails_array[i]);

		l = l->next;
	}
	change_emails_array[i] = NULL;

	// call dbus api with appropriate params
	ret = eas_gdbus_common_call (self, "sync_folder_items",
				     progress_fn, progress_data,
				     "(sussuas^as^asu)", "(sb^as^as^as^as^as^as)",
				     cancellable, error,
				     /* input parameters... */
				     self->priv->eas_client.account_uid, EAS_ITEM_MAIL,
				     sync_key_in, folder_id,
				     time_window,
				     /* Note type 'as' for this, so that NULL doesn't cause a crash */
				     NULL,			// add items - always NULL for email
				     /* Ick. It crashes if we pass NULL. Hack around it... */
				     delete_emails_array?:&change_emails_array[i],	// delete items.
				     change_emails_array,	// change items
				     0,				// request ID
				     /* output parameters... */
				     sync_key_out, more_available,
				     &created_emailinfo_array,
				     &deleted_emailinfo_array,
				     &changed_emailinfo_array,
				     NULL, NULL, &ret_failed_changes_array);

	g_debug ("dbus call returned");

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

		g_strfreev (created_emailinfo_array);
		g_strfreev (changed_emailinfo_array);
		g_strfreev (deleted_emailinfo_array);

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
