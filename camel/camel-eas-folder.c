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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include "../libeasmail/src/libeasmail.h"
#include "camel-eas-compat.h"
#include "camel-eas-folder.h"
#include "camel-eas-private.h"
#include "camel-eas-store.h"
#include "camel-eas-summary.h"
#include "camel-eas-utils.h"

#define CAMEL_EAS_FOLDER_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_EAS_FOLDER, CamelEasFolderPrivate))

struct _CamelEasFolderPrivate {
	GMutex *search_lock;	/* for locking the search object */
	GStaticRecMutex cache_lock;	/* for locking the cache object */

	/* For syncronizing refresh_info/sync_changes */
	gboolean refreshing;
	gboolean fetch_pending;
	GMutex *state_lock;
	GCond *fetch_cond;
	GHashTable *uid_eflags;
};

#define d(x)

G_DEFINE_TYPE (CamelEasFolder, camel_eas_folder, CAMEL_TYPE_OFFLINE_FOLDER)

static gchar *
eas_get_filename (CamelFolder *folder, const gchar *uid, GError **error)
{
	CamelEasFolder *eas_folder = CAMEL_EAS_FOLDER(folder);

	return camel_data_cache_get_filename (eas_folder->cache, "cache", uid, error);
}

#if ! EDS_CHECK_VERSION(2,33,0)
static gboolean camel_data_wrapper_construct_from_stream_sync(CamelDataWrapper *data_wrapper,
							     CamelStream *stream,
							     GCancellable *cancellable,
							     GError **error)
{
	/* In 2.32 this returns an int, which is zero for success */
	return !camel_data_wrapper_construct_from_stream(data_wrapper, stream, error);
}

#endif


static CamelMimeMessage *
camel_eas_folder_get_message_from_cache (CamelEasFolder *eas_folder, const gchar *uid, GCancellable *cancellable, GError **error)
{
	CamelStream *stream;
	CamelMimeMessage *msg;
	CamelEasFolderPrivate *priv;

	priv = eas_folder->priv;

	g_static_rec_mutex_lock (&priv->cache_lock);
	stream = camel_data_cache_get (eas_folder->cache, "cur", uid, error);
	if (!stream) {
		g_static_rec_mutex_unlock (&priv->cache_lock);
		return NULL;
	}

	msg = camel_mime_message_new ();

	if (!camel_data_wrapper_construct_from_stream_sync (
				(CamelDataWrapper *)msg, stream, cancellable, error)) {
		g_object_unref (msg);
		msg = NULL;
	}

	g_static_rec_mutex_unlock (&priv->cache_lock);
	g_object_unref (stream);

	return msg;
}
#if 0
static CamelMimeMessage *
camel_eas_folder_get_message (CamelFolder *folder, const gchar *uid, gint pri, GCancellable *cancellable, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	EEasConnection *cnc;
	CamelEasStore *eas_store;
	const gchar *mime_content;
	CamelMimeMessage *message = NULL;
	CamelStream *tmp_stream = NULL;
	GSList *ids = NULL, *items = NULL;
	gchar *mime_dir;
	gchar *cache_file;
	gchar *dir;
	const gchar *temp;
	gpointer progress_data;
	gboolean res;
	gchar *mime_fname_new = NULL;

	eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);
	eas_folder = (CamelEasFolder *) folder;
	priv = eas_folder->priv;

	if (!camel_eas_store_connected (eas_store, error))
		return NULL;

	g_mutex_lock (priv->state_lock);

	/* If another thread is already fetching this message, wait for it */

	/* FIXME: We might end up refetching a message anyway, if another
	   thread has already finished fetching it by the time we get to
	   this point in the code — eas_folder_get_message_sync() doesn't
	   hold any locks when it calls get_message_from_cache() and then
	   falls back to this function. */
	if (g_hash_table_lookup (priv->uid_eflags, uid)) {
		do {
			g_cond_wait (priv->fetch_cond, priv->state_lock);
		} while (g_hash_table_lookup (priv->uid_eflags, uid));

		g_mutex_unlock (priv->state_lock);

		message = camel_eas_folder_get_message_from_cache (eas_folder, uid, cancellable, error);
		return message;
	}

	/* Because we're using this as a form of mutex, we *know* that
	   we won't be inserting where an entry already exists. So it's
	   OK to insert uid itself, not g_strdup (uid) */
	g_hash_table_insert (priv->uid_eflags, (gchar *)uid, (gchar *)uid);
	g_mutex_unlock (priv->state_lock);

	cnc = camel_eas_store_get_connection (eas_store);
	ids = g_slist_append (ids, (gchar *) uid);
	EVO3(progress_data = cancellable);
	EVO2(progress_data = camel_operation_registered ());

	mime_dir = g_build_filename (camel_data_cache_get_path (eas_folder->cache),
				     "mimecontent", NULL);

	if (g_access (mime_dir, F_OK) == -1 &&
	    g_mkdir_with_parents (mime_dir, 0700) == -1) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Unable to create cache path"));
		g_free (mime_dir);
		goto exit;
	}

	res = e_eas_connection_get_items (cnc, pri, ids, "IdOnly", "item:MimeContent",
					  TRUE, mime_dir,
					  &items,
					  (ESoapProgressFn)camel_operation_progress,
					  progress_data,
					  cancellable, error);
	g_free (mime_dir);

	if (!res)
		goto exit;

	/* The mime_content actually contains the *filename*, due to the
	   streaming hack in ESoapMessage */
	mime_content = e_eas_item_get_mime_content (items->data);

	/* Exchange returns random UID for associated calendar item, which has no way
	   to match with calendar components saved in calendar cache. So manually get
	   AssociatedCalendarItemId, replace the random UID with this ItemId,
	   And save updated message data to a new temp file */
	if (e_eas_item_get_item_type (items->data) == E_EAS_ITEM_TYPE_MEETING_REQUEST ||
		e_eas_item_get_item_type (items->data) == E_EAS_ITEM_TYPE_MEETING_CANCELLATION ||
		e_eas_item_get_item_type (items->data) == E_EAS_ITEM_TYPE_MEETING_MESSAGE ||
		e_eas_item_get_item_type (items->data) == E_EAS_ITEM_TYPE_MEETING_RESPONSE) {
		GSList *items_req = NULL;
		const EasId *associated_calendar_id;

		// Get AssociatedCalendarItemId with second get_items call
		res = e_eas_connection_get_items (cnc, pri, ids, "IdOnly", "meeting:AssociatedCalendarItemId",
						  FALSE, NULL,
						  &items_req,
						  (ESoapProgressFn)camel_operation_progress,
						  progress_data,
						  cancellable, error);
		if (!res) {
			if (items_req) {
				g_object_unref (items_req->data);
				g_slist_free (items_req);
			}
			goto exit;
		}
		associated_calendar_id = e_eas_item_get_associated_calendar_item_id (items_req->data);
		/*In case of non-exchange based meetings invites the calendar backend have to create the meeting*/
		if (associated_calendar_id) {
			mime_fname_new = eas_update_mgtrequest_mime_calendar_itemid (mime_content,
										     associated_calendar_id,
										     error);
		}
		if (mime_fname_new)
			mime_content = (const gchar *) mime_fname_new;

		if (items_req) {
			g_object_unref (items_req->data);
			g_slist_free (items_req);
		}
	}

	cache_file = camel_data_cache_get_filename  (eas_folder->cache, "cur",
						     uid, error);
	temp = g_strrstr (cache_file, "/");
	dir = g_strndup (cache_file, temp - cache_file);

	if (g_mkdir_with_parents (dir, 0700) == -1) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Unable to create cache path"));
		g_free (dir);
		g_free (cache_file);
		goto exit;
	}
	g_free (dir);

	if (g_rename (mime_content, cache_file) != 0) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Failed to move message cache file"));
		g_free (cache_file);
		goto exit;
	}
	g_free (cache_file);

	message = camel_eas_folder_get_message_from_cache (eas_folder, uid, cancellable, error);

exit:
	g_mutex_lock (priv->state_lock);
	g_hash_table_remove (priv->uid_eflags, uid);
	g_mutex_unlock (priv->state_lock);
	g_cond_broadcast (priv->fetch_cond);

	if (!message && !error)
		g_set_error (
			error, CAMEL_ERROR, 1,
			"Could not retrieve the message");
	if (ids)
		g_slist_free (ids);
	if (items) {
		g_object_unref (items->data);
		g_slist_free (items);
	}

	if (tmp_stream)
		g_object_unref (tmp_stream);

	if (mime_fname_new)
		g_free (mime_fname_new);

	return message;
}
#endif
/* Get the message from cache if available otherwise get it from server */
static CamelMimeMessage *
eas_folder_get_message_sync (CamelFolder *folder, const gchar *uid, EVO3(GCancellable *cancellable,) GError **error )
{
	CamelMimeMessage *message;
	EVO2(GCancellable *cancellable = NULL);

	message = camel_eas_folder_get_message_from_cache ((CamelEasFolder *)folder, uid, cancellable, error);
	//	if (!message)
	//	message = camel_eas_folder_get_message (folder, uid, EAS_ITEM_HIGH, cancellable, error);

	return message;
}

static GPtrArray *
eas_folder_search_by_expression (CamelFolder *folder, const gchar *expression, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	GPtrArray *matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	g_mutex_lock (priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_search (eas_folder->search, expression, NULL, error);

	g_mutex_unlock (priv->search_lock);

	return matches;
}

static guint32
eas_folder_count_by_expression (CamelFolder *folder, const gchar *expression, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	guint32 matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	g_mutex_lock (priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_count (eas_folder->search, expression, error);

	g_mutex_unlock (priv->search_lock);

	return matches;
}

static GPtrArray *
eas_folder_search_by_uids(CamelFolder *folder, const gchar *expression, GPtrArray *uids, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	GPtrArray *matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	if (uids->len == 0)
		return g_ptr_array_new ();

	g_mutex_lock (priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_search (eas_folder->search, expression, uids, error);

	g_mutex_unlock (priv->search_lock);

	return matches;
}

static void
eas_folder_search_free (CamelFolder *folder, GPtrArray *uids)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	g_return_if_fail (eas_folder->search);

	g_mutex_lock (priv->search_lock);

	camel_folder_search_free_result (eas_folder->search, uids);

	g_mutex_unlock (priv->search_lock);

	return;
}

/********************* folder functions*************************/


static gboolean
eas_synchronize_sync (CamelFolder *folder, gboolean expunge, EVO3(GCancellable *cancellable,) GError **error)
{
	CamelEasStore *eas_store;
	GPtrArray *uids;
	GSList *mi_list = NULL, *deleted_uids = NULL;
	int mi_list_len = 0;
	gboolean success = TRUE;
	int i;
	EVO2(GCancellable *cancellable = NULL);

	eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);

	if (!camel_eas_store_connected (eas_store, error))
		return FALSE;

	uids = camel_folder_summary_get_changed (folder->summary);
	if (!uids->len) {
		camel_folder_free_uids (folder, uids);
		return TRUE;
	}
#if 0
	for (i = 0; success && i < uids->len; i++) {
		guint32 flags_changed;
		CamelEasMessageInfo *mi = (void *)camel_folder_summary_uid (folder->summary, uids->pdata[i]);
		if (!mi)
			continue;

		flags_changed = mi->server_flags ^ mi->info.flags;

		/* Exchange doesn't seem to have a sane representation
		   for most flags — not even replied/forwarded. */
		if (flags_changed & (CAMEL_MESSAGE_SEEN|CAMEL_MESSAGE_ANSWERED|CAMEL_MESSAGE_FORWARDED)) {
			mi_list = g_slist_append (mi_list, mi);
			mi_list_len++;
		} else if (flags_changed & CAMEL_MESSAGE_DELETED) {
			deleted_uids = g_slist_prepend (deleted_uids, (gpointer) camel_pstring_strdup (uids->pdata [i]));
			camel_message_info_free (mi);
		}

		if (mi_list_len == EAS_MAX_FETCH_COUNT) {
			success = eas_sync_mi_flags (folder, mi_list, cancellable, error);
			mi_list = NULL;
			mi_list_len = 0;
		}
	}
	
	if (mi_list_len)
		success = eas_sync_mi_flags (folder, mi_list, cancellable, error);

	if (deleted_uids)
		success = eas_delete_messages (folder, deleted_uids, FALSE, cancellable, error);

	camel_folder_free_uids (folder, uids);
#endif
	return success;
}

CamelFolder *
camel_eas_folder_new (CamelStore *store, const gchar *folder_name, const gchar *folder_dir, GCancellable *cancellable, GError **error)
{
        CamelFolder *folder;
        CamelEasFolder *eas_folder;
        gchar *summary_file, *state_file;
        const gchar *short_name;

        short_name = strrchr (folder_name, '/');
        if (!short_name)
                short_name = folder_name;

        folder = g_object_new (
                CAMEL_TYPE_EAS_FOLDER,
                "name", short_name, "full-name", folder_name,
                "parent_store", store, NULL);

        eas_folder = CAMEL_EAS_FOLDER(folder);

        summary_file = g_build_filename (folder_dir, "summary", NULL);
        folder->summary = camel_eas_summary_new (folder, summary_file);
        g_free(summary_file);

        if (!folder->summary) {
                g_object_unref (CAMEL_OBJECT (folder));
                g_set_error (
                        error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
                        _("Could not load summary for %s"), folder_name);
                return NULL;
        }

        /* set/load persistent state */
        state_file = g_build_filename (folder_dir, "cmeta", NULL);
        camel_object_set_state_filename (CAMEL_OBJECT (folder), state_file);
        camel_object_state_read (CAMEL_OBJECT (folder));
        g_free(state_file);

        eas_folder->cache = camel_data_cache_new (folder_dir, error);
        if (!eas_folder->cache) {
                g_object_unref (folder);
                return NULL;
        }

        if (!g_ascii_strcasecmp (folder_name, "Inbox")) {
                if (camel_url_get_param (((CamelService *) store)->url, "filter"))
                        folder->folder_flags |= CAMEL_FOLDER_FILTER_RECENT;
        }

        eas_folder->search = camel_folder_search_new ();
        if (!eas_folder->search) {
                g_object_unref (folder);
                return NULL;
        }

        return folder;
}


static gboolean
eas_refresh_info_sync (CamelFolder *folder, EVO3(GCancellable *cancellable,) GError **error)
{
        CamelEasFolder *eas_folder;
        CamelEasFolderPrivate *priv;
        EasEmailHandler *handler;
        CamelEasStore *eas_store;
        const gchar *full_name;
        gchar *id;
        gchar *sync_state;
	gboolean more_available = TRUE;
	GSList *items_created = NULL, *items_updated = NULL, *items_deleted = NULL;
        GError *rerror = NULL;
        EVO2(GCancellable *cancellable = NULL);

        full_name = camel_folder_get_full_name (folder);
        eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);

        eas_folder = (CamelEasFolder *) folder;
        priv = eas_folder->priv;

        if (!camel_eas_store_connected (eas_store, error))
                return FALSE;

        g_mutex_lock (priv->state_lock);

        if (priv->refreshing) {
                g_mutex_unlock (priv->state_lock);
                return TRUE;
        }

        priv->refreshing = TRUE;
        g_mutex_unlock (priv->state_lock);

        handler = camel_eas_store_get_handler (eas_store);
        id = camel_eas_store_summary_get_folder_id_from_name
                                                (eas_store->summary,
                                                 full_name);

        sync_state = ((CamelEasSummary *) folder->summary)->sync_state;
	do {
		guint total, unread;

		if (!eas_mail_handler_sync_folder_email_info (handler, sync_state, id,
							      &items_created,
							      &items_updated, &items_deleted,
							      &more_available, error)) {
			return FALSE;
		}
		if (items_deleted)
			camel_eas_utils_sync_deleted_items (eas_folder, items_deleted);

		if (items_created)
			camel_eas_utils_sync_created_items (eas_folder, items_created);

		if (items_updated)
			camel_eas_utils_sync_updated_items (eas_folder, items_updated);

                total = camel_folder_summary_count (folder->summary);
                unread = folder->summary->unread_count;

                camel_eas_store_summary_set_folder_total (eas_store->summary, id, total);
                camel_eas_store_summary_set_folder_unread (eas_store->summary, id, unread);
                camel_eas_store_summary_save (eas_store->summary, NULL);

                camel_folder_summary_save_to_db (folder->summary, NULL);

        } while (!rerror && more_available);

        if (rerror)
                g_propagate_error (error, rerror);

        g_mutex_lock (priv->state_lock);
        priv->refreshing = FALSE;
        g_mutex_unlock (priv->state_lock);
        if (sync_state != ((CamelEasSummary *) folder->summary)->sync_state)
                g_free(sync_state);
        g_object_unref (handler);
        g_free (id);

	return TRUE;
}

static gboolean
eas_append_message_sync (CamelFolder *folder, CamelMimeMessage *message,
	 		 EVO2(const) CamelMessageInfo *info,
			 gchar **appended_uid,
	 		 EVO3(GCancellable *cancellable,) GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Writing message to mail store not possible in ActiveSync"));
	return FALSE;
}

/* move messages */
static gboolean
eas_transfer_messages_to_sync	(CamelFolder *source,
				 GPtrArray *uids,
				 CamelFolder *destination,
				 EVO2(GPtrArray **transferred_uids,)
				 gboolean delete_originals,
				 EVO3(GPtrArray **transferred_uids,)
				 EVO3(GCancellable *cancellable,)
				 GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Moving messages not yet implemented"));
	return FALSE;
}

static gboolean
eas_delete_messages (CamelFolder *folder, GSList *deleted_items, gboolean expunge, GCancellable *cancellable, GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Deleting message not yet implemented"));
	return FALSE;
}

static gboolean
eas_expunge_sync (CamelFolder *folder, EVO3(GCancellable *cancellable,) GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Expunging folder not yet implemented"));
	return FALSE;
}

static gint
eas_cmp_uids (CamelFolder *folder, const gchar *uid1, const gchar *uid2)
{
	g_return_val_if_fail (uid1 != NULL, 0);
	g_return_val_if_fail (uid2 != NULL, 0);

	return strcmp (uid1, uid2);
}

static void
eas_folder_dispose (GObject *object)
{
	CamelEasFolder *eas_folder = CAMEL_EAS_FOLDER (object);

	if (eas_folder->cache != NULL) {
		g_object_unref (eas_folder->cache);
		eas_folder->cache = NULL;
	}

	if (eas_folder->search != NULL) {
		g_object_unref (eas_folder->search);
		eas_folder->search = NULL;
	}

	g_mutex_free (eas_folder->priv->search_lock);
	g_hash_table_destroy (eas_folder->priv->uid_eflags);
	g_cond_free (eas_folder->priv->fetch_cond);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (camel_eas_folder_parent_class)->dispose (object);
}

static void
eas_folder_constructed (GObject *object)
{
	CamelFolder *folder;
	CamelStore *parent_store;
	CamelURL *url;
	const gchar *full_name;
	gchar *description;

	folder = CAMEL_FOLDER (object);
	full_name = camel_folder_get_full_name (folder);
	parent_store = camel_folder_get_parent_store (folder);
	url = CAMEL_SERVICE (parent_store)->url;

	description = g_strdup_printf (
		"%s@%s:%s", url->user, url->host, full_name);
	camel_folder_set_description (folder, description);
	g_free (description);
}

static void
camel_eas_folder_class_init (CamelEasFolderClass *class)
{
	GObjectClass *object_class;
	CamelFolderClass *folder_class;

	g_type_class_add_private (class, sizeof (CamelEasFolderPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = eas_folder_dispose;
	object_class->constructed = eas_folder_constructed;

	folder_class = CAMEL_FOLDER_CLASS (class);
	folder_class->EVO3_sync(get_message) = eas_folder_get_message_sync;
	folder_class->search_by_expression = eas_folder_search_by_expression;
	folder_class->count_by_expression = eas_folder_count_by_expression;
	folder_class->cmp_uids = eas_cmp_uids;
	folder_class->search_by_uids = eas_folder_search_by_uids;
	folder_class->search_free = eas_folder_search_free;
	folder_class->EVO3_sync(append_message) = eas_append_message_sync;
	folder_class->EVO3_sync(refresh_info) = eas_refresh_info_sync;
	EVO3(folder_class->synchronize_sync = eas_synchronize_sync);
	EVO2(folder_class->sync = eas_synchronize_sync);
	folder_class->EVO3_sync(expunge) = eas_expunge_sync;
	folder_class->EVO3_sync(transfer_messages_to) = eas_transfer_messages_to_sync;
	folder_class->get_filename = eas_get_filename;
}

static void
camel_eas_folder_init (CamelEasFolder *eas_folder)
{
	CamelFolder *folder = CAMEL_FOLDER (eas_folder);

	eas_folder->priv = CAMEL_EAS_FOLDER_GET_PRIVATE (eas_folder);

	folder->permanent_flags = CAMEL_MESSAGE_ANSWERED | CAMEL_MESSAGE_DELETED |
		CAMEL_MESSAGE_DRAFT | CAMEL_MESSAGE_FLAGGED | CAMEL_MESSAGE_SEEN |
		CAMEL_MESSAGE_FORWARDED;

	folder->folder_flags = CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY | CAMEL_FOLDER_HAS_SEARCH_CAPABILITY;

	eas_folder->priv->search_lock = g_mutex_new ();
	eas_folder->priv->state_lock = g_mutex_new ();
	g_static_rec_mutex_init(&eas_folder->priv->cache_lock);

	eas_folder->priv->refreshing = FALSE;

	eas_folder->priv->fetch_cond = g_cond_new ();
	eas_folder->priv->uid_eflags = g_hash_table_new (g_str_hash, g_str_equal);
	camel_folder_set_lock_async (folder, TRUE);
}

/** End **/
