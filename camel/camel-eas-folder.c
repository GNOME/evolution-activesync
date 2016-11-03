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
#include <gio/gio.h>

#include <dbus/dbus-glib.h>

#include <libeasmail.h>
#include "camel-eas-folder.h"
#include "camel-eas-private.h"
#include "camel-eas-store.h"
#include "camel-eas-summary.h"
#include "camel-eas-utils.h"

#define CAMEL_EAS_FOLDER_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_EAS_FOLDER, CamelEasFolderPrivate))

struct _CamelEasFolderPrivate {
	gchar *server_id;
	GMutex search_lock;	/* for locking the search object */
	GRecMutex cache_lock;	/* for locking the cache object */

	/* For syncronizing refresh_info/sync_changes */
	gboolean refreshing;
	gboolean fetch_pending;
	GMutex state_lock;
	GMutex server_lock;
	GCond fetch_cond;
	GHashTable *uid_eflags;
};

#define d(x)

G_DEFINE_TYPE (CamelEasFolder, camel_eas_folder, CAMEL_TYPE_OFFLINE_FOLDER)

static gchar *
eas_get_filename (CamelFolder *folder, const gchar *uid, GError **error)
{
	CamelEasFolder *eas_folder = CAMEL_EAS_FOLDER(folder);

	return camel_data_cache_get_filename (eas_folder->cache, "cache", uid);
}

static CamelMimeMessage *
camel_eas_folder_get_message_from_cache (CamelEasFolder *eas_folder, const gchar *uid, GCancellable *cancellable, GError **error)
{
	CamelStream *stream = NULL;
	GIOStream *base_stream;
	CamelMimeMessage *msg;
	CamelEasFolderPrivate *priv;

	priv = eas_folder->priv;

	g_rec_mutex_lock (&priv->cache_lock);
	base_stream = camel_data_cache_get (eas_folder->cache, "cur", uid,
					    error);
	if (!base_stream) {
		g_rec_mutex_unlock (&priv->cache_lock);
		return NULL;
	}
	stream = camel_stream_new (base_stream);
	g_object_unref (base_stream);

	msg = camel_mime_message_new ();

	if (!camel_data_wrapper_construct_from_stream_sync (
				(CamelDataWrapper *)msg, stream, cancellable, error)) {
		g_object_unref (msg);
		msg = NULL;
	}

	g_rec_mutex_unlock (&priv->cache_lock);
	g_object_unref (stream);

	return msg;
}

static CamelMimeMessage *
camel_eas_folder_get_message (CamelFolder *folder, const gchar *uid, 
			      GCancellable *cancellable, GError **error)
{
	gpointer progress_data;
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	EasEmailHandler *handler;
	CamelEasStore *eas_store;
	gchar *mime_content;
	CamelMimeMessage *message = NULL;
	CamelStream *tmp_stream = NULL;
	GSList *ids = NULL, *items = NULL;
	gchar *mime_dir;
	gchar *cache_file;
	gchar *dir;
	const gchar *temp;
	gboolean res;
	gchar *mime_fname_new = NULL;

	progress_data = cancellable;

	eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);
	eas_folder = (CamelEasFolder *) folder;
	priv = eas_folder->priv;

	if (!camel_eas_store_connected (eas_store, cancellable, error))
		return NULL;

	g_mutex_lock (&priv->state_lock);

	/* If another thread is already fetching this message, wait for it */

	/* FIXME: We might end up refetching a message anyway, if another
	   thread has already finished fetching it by the time we get to
	   this point in the code — eas_folder_get_message_sync() doesn't
	   hold any locks when it calls get_message_from_cache() and then
	   falls back to this function. */
	if (g_hash_table_lookup (priv->uid_eflags, uid)) {
		do {
			g_cond_wait (&priv->fetch_cond, &priv->state_lock);
		} while (g_hash_table_lookup (priv->uid_eflags, uid));

		g_mutex_unlock (&priv->state_lock);

		message = camel_eas_folder_get_message_from_cache (eas_folder, uid, cancellable, error);
		return message;
	}

	/* Because we're using this as a form of mutex, we *know* that
	   we won't be inserting where an entry already exists. So it's
	   OK to insert uid itself, not g_strdup (uid) */
	g_hash_table_insert (priv->uid_eflags, (gchar *)uid, (gchar *)uid);
	g_mutex_unlock (&priv->state_lock);

	handler = camel_eas_store_get_handler (eas_store);

	mime_dir = g_build_filename (camel_data_cache_get_path (eas_folder->cache),
				     "mimecontent", NULL);

	if (g_access (mime_dir, F_OK) == -1 &&
	    g_mkdir_with_parents (mime_dir, 0700) == -1) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Unable to create cache path"));
		g_free (mime_dir);
		goto exit;
	}

	g_mutex_lock (&priv->server_lock);
	res = eas_mail_handler_fetch_email_body (handler, priv->server_id, uid, mime_dir,
						 (EasProgressFn)camel_operation_progress, progress_data, NULL, error);
	g_mutex_unlock (&priv->server_lock);

	if (!res) {
		g_free (mime_dir);
		goto exit;
	}

	/* The mime_content actually contains the *filename*, due to the
	   streaming hack in ESoapMessage */
	mime_content = g_build_filename (mime_dir, uid, NULL);
	g_free (mime_dir);
	cache_file = camel_data_cache_get_filename  (eas_folder->cache, "cur",
						     uid);
	temp = g_strrstr (cache_file, "/");
	dir = g_strndup (cache_file, temp - cache_file);

	if (g_mkdir_with_parents (dir, 0700) == -1) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Unable to create cache path"));
		g_free (dir);
		g_free (cache_file);
		g_free (mime_content);
		goto exit;
	}
	g_free (dir);

	if (g_rename (mime_content, cache_file) != 0) {
		g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			     _("Failed to move message cache file"));
		g_free (cache_file);
		g_free (mime_content);
		goto exit;
	}
	g_free (cache_file);
	g_free (mime_content);

	message = camel_eas_folder_get_message_from_cache (eas_folder, uid, cancellable, error);

exit:
	g_mutex_lock (&priv->state_lock);
	g_hash_table_remove (priv->uid_eflags, uid);
	g_mutex_unlock (&priv->state_lock);
	g_cond_broadcast (&priv->fetch_cond);

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

static guint32
eas_folder_get_permanent_flags (CamelFolder *folder)
{
	return CAMEL_MESSAGE_ANSWERED |
		CAMEL_MESSAGE_DELETED |
		CAMEL_MESSAGE_DRAFT |
		CAMEL_MESSAGE_FLAGGED |
		CAMEL_MESSAGE_SEEN |
		CAMEL_MESSAGE_FORWARDED;
}

/* Get the message from cache if available otherwise get it from server */
static CamelMimeMessage *
eas_folder_get_message_sync (CamelFolder *folder, const gchar *uid, GCancellable *cancellable, GError **error )
{
	CamelMimeMessage *message;

	message = camel_eas_folder_get_message_from_cache ((CamelEasFolder *)folder, uid, cancellable, NULL);
	if (!message)
		message = camel_eas_folder_get_message (folder, uid, cancellable, error);

	return message;
}

static GPtrArray *
eas_folder_search_by_expression (CamelFolder *folder, const gchar *expression, GCancellable *cancellable, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	GPtrArray *matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	g_mutex_lock (&priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_search (eas_folder->search, expression, NULL, cancellable, error);

	g_mutex_unlock (&priv->search_lock);

	return matches;
}

static guint32
eas_folder_count_by_expression (CamelFolder *folder, const gchar *expression, GCancellable *cancellable, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	guint32 matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	g_mutex_lock (&priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_count (eas_folder->search, expression, cancellable, error);

	g_mutex_unlock (&priv->search_lock);

	return matches;
}

static GPtrArray *
eas_folder_search_by_uids(CamelFolder *folder, const gchar *expression, GPtrArray *uids, 
			  GCancellable *cancellable, GError **error)
{
	CamelEasFolder *eas_folder;
	CamelEasFolderPrivate *priv;
	GPtrArray *matches;

	eas_folder = CAMEL_EAS_FOLDER (folder);
	priv = eas_folder->priv;

	if (uids->len == 0)
		return g_ptr_array_new ();

	g_mutex_lock (&priv->search_lock);

	camel_folder_search_set_folder (eas_folder->search, folder);
	matches = camel_folder_search_search (eas_folder->search, expression, uids, cancellable, error);

	g_mutex_unlock (&priv->search_lock);

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

	g_mutex_lock (&priv->search_lock);

	camel_folder_search_free_result (eas_folder->search, uids);

	g_mutex_unlock (&priv->search_lock);

	return;
}

/********************* folder functions*************************/

static gboolean
eas_delete_messages (CamelFolder *folder, GSList *deleted_uids, gboolean expunge, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store;
	CamelFolderSummary *folder_summary;
	EasEmailHandler *handler;
	gboolean success;
        CamelEasFolderPrivate *priv = CAMEL_EAS_FOLDER(folder)->priv;

	eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);
	folder_summary = camel_folder_get_folder_summary (folder);
        handler = camel_eas_store_get_handler (eas_store);

	g_mutex_lock (&priv->server_lock);
	success = eas_mail_handler_delete_email (handler,
						 ((CamelEasSummary *) folder_summary)->sync_state,
						 priv->server_id, deleted_uids, NULL, error);
	g_mutex_unlock (&priv->server_lock);
	if (success) {
		CamelFolderChangeInfo *changes = camel_folder_change_info_new ();
		GSList *l;
		for (l = deleted_uids; l != NULL; l = g_slist_next (l)) {
			gchar *uid = l->data;
			camel_folder_summary_lock (folder_summary);
			camel_folder_change_info_remove_uid (changes, uid);
			camel_data_cache_remove (CAMEL_EAS_FOLDER (folder)->cache, "cache", uid, NULL);
			camel_folder_summary_unlock (folder_summary);
		}
		camel_folder_changed (folder, changes);
		camel_folder_change_info_free (changes);
	}

	g_slist_foreach (deleted_uids, (GFunc) g_free, NULL);
	g_slist_free (deleted_uids);

	return success;
}

static gboolean
eas_synchronize_sync (CamelFolder *folder, gboolean expunge, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store;
	CamelFolderSummary *folder_summary;
	EasEmailHandler *handler;
	GPtrArray *uids;
	GSList *item_list = NULL, *deleted_uids = NULL;
	GSList *changing_mis = NULL, *l;
	int item_list_len = 0;
	gboolean success = TRUE;
        CamelEasFolderPrivate *priv = CAMEL_EAS_FOLDER(folder)->priv;
	int i;

	eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);
	folder_summary = camel_folder_get_folder_summary (folder);
        handler = camel_eas_store_get_handler (eas_store);

	if (!camel_eas_store_connected (eas_store, cancellable, error))
		return FALSE;

	uids = camel_folder_summary_get_changed (folder_summary);
	if (!uids->len) {
		camel_folder_free_uids (folder, uids);
		return TRUE;
	}

	i = 0;
 more:
	for ( ; success && i < uids->len && item_list_len < 25; i++) {
		guint32 flags_changed;
		CamelMessageInfo *mi = camel_folder_summary_get (folder_summary, uids->pdata[i]);
		if (!mi)
			continue;

		flags_changed = camel_eas_message_info_get_server_flags (CAMEL_EAS_MESSAGE_INFO (mi)) ^ camel_message_info_get_flags (mi);

		/* Exchange doesn't seem to have a sane representation
		   for most flags — not even replied/forwarded. */
		if (flags_changed & (CAMEL_MESSAGE_SEEN/*|CAMEL_MESSAGE_ANSWERED|CAMEL_MESSAGE_FORWARDED*/)) {
			EasEmailInfo *item = eas_email_info_new();
			item->server_id = g_strdup (uids->pdata[i]);
			if (flags_changed & CAMEL_MESSAGE_SEEN) {
				item->flags |= EAS_VALID_READ;
				if (camel_message_info_get_flags (mi) & CAMEL_MESSAGE_SEEN)
					item->flags |= EAS_EMAIL_READ;
			}
#if 0 /* ActiveSync doesn't let you update this */
			if (flags_changed & CAMEL_MESSAGE_FLAGGED) {
				item->flags |= EAS_VALID_IMPORTANCE;
				if (camel_message_info_get_flags (mi) & CAMEL_MESSAGE_FLAGGED)
					item->importance = EAS_IMPORTANCE_HIGH;
				else
					item->importance = EAS_IMPORTANCE_LOW;
			}
#endif
			changing_mis = g_slist_append (changing_mis, mi);
			item_list = g_slist_append (item_list, item);
			item_list_len++;
		} else if (flags_changed & CAMEL_MESSAGE_DELETED) {
			deleted_uids = g_slist_prepend (deleted_uids, (gpointer) camel_pstring_strdup (uids->pdata [i]));
			g_clear_object (&mi);
		}
	}

	/* Don't do too many at once */
	if (item_list_len) {
		g_mutex_lock (&priv->server_lock);
		success = eas_mail_handler_update_email (handler,
							 ((CamelEasSummary *) folder_summary)->sync_state,
							 priv->server_id, item_list, NULL, error);
		for (l = changing_mis; l; l = l->next) {
			CamelMessageInfo *mi = l->data;
			if (success) {
				camel_eas_message_info_set_server_flags (CAMEL_EAS_MESSAGE_INFO (mi), camel_message_info_get_flags (mi));

			}
			g_clear_object (&mi);
		}
		g_slist_free (changing_mis);
		changing_mis = NULL;

		g_mutex_unlock (&priv->server_lock);
		g_slist_foreach (item_list, (GFunc) g_object_unref, NULL);
		g_slist_free (item_list);
		item_list = NULL;
		item_list_len = 0;
	}
	/* If we broke out of the loop just because we had a batch to
	   send already, not because we're done, then keep going */
	if (success && i < uids->len)
		goto more;

	if (deleted_uids)
		success = eas_delete_messages (folder, deleted_uids, FALSE, cancellable, error);

	camel_folder_free_uids (folder, uids);

	return success;
}


CamelFolder *
camel_eas_folder_new (CamelStore *store, const gchar *folder_name, const gchar *folder_dir, gchar *folder_id, GCancellable *cancellable, GError **error)
{
        CamelFolder *folder;
	CamelFolderSummary *folder_summary;
        CamelEasFolder *eas_folder;
        gchar *summary_file, *state_file;
        const gchar *short_name;

        short_name = strrchr (folder_name, '/');
        if (!short_name)
                short_name = folder_name;
	else
		short_name++;

        folder = g_object_new (
                CAMEL_TYPE_EAS_FOLDER,
                "display_name", short_name, "full-name", folder_name,
                "parent_store", store, NULL);

        eas_folder = CAMEL_EAS_FOLDER(folder);

        summary_file = g_build_filename (folder_dir, "summary", NULL);
        folder_summary = camel_eas_summary_new (folder, summary_file);
        g_free (summary_file);

        if (!folder_summary) {
                g_object_unref (folder);
                g_set_error (
                        error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
                        _("Could not load summary for %s"), folder_name);
                return NULL;
        }

	camel_folder_take_folder_summary (folder, folder_summary);

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
		CamelStoreSettings *settings = CAMEL_STORE_SETTINGS (camel_service_ref_settings (CAMEL_SERVICE (store)));

                if (camel_store_settings_get_filter_inbox (settings))
                        camel_folder_set_flags (folder, camel_folder_get_flags (folder) | CAMEL_FOLDER_FILTER_RECENT);
		g_object_unref(settings);
        }

        eas_folder->search = camel_folder_search_new ();
        if (!eas_folder->search) {
                g_object_unref (folder);
                return NULL;
        }

	eas_folder->priv->server_id = g_strdup (folder_id);

        return folder;
}

struct sync_progress_data {
	guint estimate;
	guint fetched;
	void *operation;
};

static void eas_sync_progress (void *data, int pc)
{
	struct sync_progress_data *s = data;
	int fetched;

	if (!s->estimate)
		return;

	/* Estimate the number of items to be fetched in the running call */
	fetched = s->estimate - s->fetched;
	/* The dæmon hard-codes window size to 100. */
	if (fetched > 100)
		fetched = 100;

	/* Factor in the percentage we've been told */
	fetched *= pc;
	fetched /= 100;

	/* Add in the number that were obtained in previous calls */
	fetched += s->fetched;

	camel_operation_progress (s->operation, fetched * 100 / s->estimate);
}

static gboolean
eas_refresh_info_sync (CamelFolder *folder, GCancellable *cancellable, GError **error)
{
        CamelEasFolder *eas_folder;
        CamelEasFolderPrivate *priv;
        EasEmailHandler *handler;
        CamelEasStore *eas_store;
        gchar *sync_state;
	gboolean res = TRUE;
	gboolean more_available = TRUE;
	GSList *items_created = NULL, *items_updated = NULL, *items_deleted = NULL;
	gboolean resynced = FALSE;
	struct sync_progress_data progress_data;
	GError *local_error = NULL;

        eas_store = (CamelEasStore *) camel_folder_get_parent_store (folder);

        eas_folder = (CamelEasFolder *) folder;
        priv = eas_folder->priv;

        if (!camel_eas_store_connected (eas_store, cancellable, error))
                return FALSE;

        g_mutex_lock (&priv->state_lock);

        if (priv->refreshing) {
                g_mutex_unlock (&priv->state_lock);
                return TRUE;
        }

        priv->refreshing = TRUE;
        g_mutex_unlock (&priv->state_lock);

        handler = camel_eas_store_get_handler (eas_store);

        sync_state = ((CamelEasSummary *) camel_folder_get_folder_summary (folder))->sync_state;

	progress_data.operation = cancellable;
	progress_data.estimate = 0;
	progress_data.fetched = 0;

 resync:
	if (!strcmp (sync_state, "0")) {
		gchar *new_sync_key = NULL;
		res = eas_mail_handler_sync_folder_email (handler, sync_state, 0, priv->server_id,
							  NULL, NULL, &new_sync_key,
							  &items_created,
							  &items_updated, &items_deleted,
							  &more_available,
							  NULL, NULL,
							  cancellable, error);
		if (!res)
			goto out;
		strncpy (sync_state, new_sync_key, 64);
		g_free (new_sync_key);
	}
	res = eas_mail_handler_get_item_estimate (handler, sync_state, priv->server_id,
						  &progress_data.estimate, &local_error);
	/* We use strcasecmp() instead of dbus_g_error_has_name() because
	   the error names will probably become CamelCase when we manage
	   to auto-generate the list on the server side. */
	if (!res && !resynced &&
	    g_error_matches (local_error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_GETITEMESTIMATE_ERROR_INVALID_SYNC_KEY)) {
	invsync:
		/* Invalid sync key. Treat it like a UIDVALIDITY change in IMAP;
		   wipe the folder and start again */
		g_warning ("Invalid SyncKey!!!");
		camel_eas_utils_clear_folder (eas_folder);
		strcpy (sync_state, "0");

		/* Make it go round again. But only once; we don't want to
		   loop for ever if the server really hates us */
		resynced = TRUE;
		more_available = TRUE;

		g_clear_error (&local_error);
		goto resync;
	}
	if (!res) {
		g_propagate_error (error, local_error);
		goto out;
	}

	/* Hopefully the server doesn't lie. It is an *estimate* but estimating
	   zero when the number is non-zero would be kind of crap. But then again,
	   this *is* Exchange we're talking about... */
	if (!progress_data.estimate)
		goto out;

	do {
		gchar *new_sync_key = NULL;

		items_created = items_updated = items_deleted = NULL;

		g_mutex_lock (&priv->server_lock);
		res = eas_mail_handler_sync_folder_email (handler, sync_state, 0, priv->server_id,
							  NULL, NULL, &new_sync_key,
							  &items_created,
							  &items_updated, &items_deleted,
							  &more_available,
							  (EasProgressFn)eas_sync_progress, &progress_data,
							  cancellable, &local_error);

		/* Google has been observed (2012-05-21) to response happily to GetItemEstimate
		   (with an estimate of 1), but then complain of Invalid Sync Key on an immediately
		   subsequent Sync call with the *same* SyncKey! So handle this here too... */
		if (!res) {
			if (!resynced && !progress_data.fetched &&
			    g_error_matches (local_error, EAS_CONNECTION_ERROR,
					     EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCKEY)) {
				g_mutex_unlock (&priv->server_lock);
				goto invsync;
			}
			g_propagate_error(error, local_error);
		}

		if (new_sync_key) {
			strncpy (sync_state, new_sync_key, 64);
			g_free (new_sync_key);
		}
		g_mutex_unlock (&priv->server_lock);

		if (!res)
			break;

		if (items_deleted)
			progress_data.fetched += camel_eas_utils_sync_deleted_items (eas_folder, items_deleted);

		if (items_created)
			progress_data.fetched += camel_eas_utils_sync_created_items (eas_folder, items_created);

		if (items_updated)
			progress_data.fetched += camel_eas_utils_sync_updated_items (eas_folder, items_updated);

                camel_folder_summary_save (camel_folder_get_folder_summary (folder), NULL);

        } while (more_available);

 out:
        g_mutex_lock (&priv->state_lock);
        priv->refreshing = FALSE;
        g_mutex_unlock (&priv->state_lock);
        g_object_unref (handler);

	return res;
}

static gboolean
eas_append_message_sync (CamelFolder *folder, CamelMimeMessage *message,
	 		 CamelMessageInfo *info,
			 gchar **appended_uid,
	 		 GCancellable *cancellable, GError **error)
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
				 gboolean delete_originals,
				 GPtrArray **transferred_uids,
				 GCancellable *cancellable,
				 GError **error)
{
	g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
		     _("Moving messages not yet implemented"));
	return FALSE;
}

static gboolean
eas_expunge_sync (CamelFolder *folder, GCancellable *cancellable, GError **error)
{
	CamelEasStore *eas_store;
	CamelMessageInfo *info;
	CamelStore *parent_store;
	CamelFolderSummary *folder_summary;
	GSList *deleted_items = NULL;
	gboolean expunge = FALSE;
	gint i;
	GPtrArray *known_uids;

	parent_store = camel_folder_get_parent_store (folder);
	folder_summary = camel_folder_get_folder_summary (folder);
	eas_store = CAMEL_EAS_STORE (parent_store);

	if (!camel_eas_store_connected (eas_store, cancellable, error))
		return FALSE;

	/* On Deleted Items folder, actually delete. On others, move to Deleted Items */
	if (camel_folder_get_flags (folder) & CAMEL_FOLDER_IS_TRASH)
		expunge = TRUE;

	/* Collect UIDs of deleted messages. */
	camel_folder_summary_prepare_fetch_all (folder_summary, NULL);
	known_uids = camel_folder_summary_get_array (folder_summary);
	if (!known_uids)
		return TRUE;

	/* Collect UIDs of deleted messages. */
	for (i = 0; i < known_uids->len; i++) {
		const gchar *uid = g_ptr_array_index (known_uids, i);

		info = camel_folder_summary_get (folder_summary, uid);

		if (info && (camel_message_info_get_flags (info) & CAMEL_MESSAGE_DELETED))
			deleted_items = g_slist_prepend (deleted_items, (gpointer) camel_pstring_strdup (uid));

		g_clear_object (&info);
	}
	camel_folder_summary_free_array (known_uids);

	if (deleted_items)
		return eas_delete_messages (folder, deleted_items, expunge, cancellable, error);
	else
		return TRUE;
}

static gint
eas_cmp_uids (CamelFolder *folder, const gchar *uid1, const gchar *uid2)
{
	g_return_val_if_fail (uid1 != NULL, 0);
	g_return_val_if_fail (uid2 != NULL, 0);

	return strcmp (uid1, uid2);
}

static void
eas_folder_finalize (GObject *object)
{
	CamelEasFolder *eas_folder = CAMEL_EAS_FOLDER (object);

	g_hash_table_destroy (eas_folder->priv->uid_eflags);

	g_free (eas_folder->priv->server_id);
	eas_folder->priv->server_id = NULL;

	g_cond_clear(&eas_folder->priv->fetch_cond);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (camel_eas_folder_parent_class)->finalize (object);

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

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (camel_eas_folder_parent_class)->dispose (object);
}

static void
eas_folder_constructed (GObject *object)
{
	CamelFolder *folder = CAMEL_FOLDER (object);
	CamelStore *parent_store = camel_folder_get_parent_store (folder);
	const gchar *full_name = camel_folder_get_full_name (folder);
	gchar *description;
	const gchar *user, *host;

        CamelService *service = CAMEL_SERVICE (parent_store);
	CamelSettings *settings = camel_service_ref_settings (service);
        CamelNetworkSettings *network_settings = CAMEL_NETWORK_SETTINGS (settings);

        host = camel_network_settings_get_host (network_settings);
        user = camel_network_settings_get_user (network_settings);
	description = g_strdup_printf ("%s@%s:%s", user, host, full_name);
	camel_folder_set_description (folder, description);
	g_free (description);
	g_object_unref(settings);
}

static void
camel_eas_folder_class_init (CamelEasFolderClass *class)
{
	GObjectClass *object_class;
	CamelFolderClass *folder_class;

	g_type_class_add_private (class, sizeof (CamelEasFolderPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = eas_folder_dispose;
	object_class->finalize = eas_folder_finalize;
	object_class->constructed = eas_folder_constructed;

	folder_class = CAMEL_FOLDER_CLASS (class);
	folder_class->get_permanent_flags = eas_folder_get_permanent_flags;
	folder_class->get_message_sync = eas_folder_get_message_sync;
	folder_class->search_by_expression = eas_folder_search_by_expression;
	folder_class->count_by_expression = eas_folder_count_by_expression;
	folder_class->cmp_uids = eas_cmp_uids;
	folder_class->search_by_uids = eas_folder_search_by_uids;
	folder_class->search_free = eas_folder_search_free;
	folder_class->append_message_sync = eas_append_message_sync;
	folder_class->refresh_info_sync = eas_refresh_info_sync;
	folder_class->synchronize_sync = eas_synchronize_sync;
	folder_class->expunge_sync = eas_expunge_sync;
	folder_class->transfer_messages_to_sync = eas_transfer_messages_to_sync;
	folder_class->get_filename = eas_get_filename;
}

static void
camel_eas_folder_init (CamelEasFolder *eas_folder)
{
	CamelFolder *folder = CAMEL_FOLDER (eas_folder);

	eas_folder->priv = CAMEL_EAS_FOLDER_GET_PRIVATE (eas_folder);

	camel_folder_set_flags (folder, CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY);

	g_mutex_init(&eas_folder->priv->search_lock);
	g_mutex_init(&eas_folder->priv->state_lock);
	g_mutex_init(&eas_folder->priv->server_lock);
	g_rec_mutex_init(&eas_folder->priv->cache_lock);

	eas_folder->priv->refreshing = FALSE;

	g_cond_init(&eas_folder->priv->fetch_cond);
	eas_folder->priv->uid_eflags = g_hash_table_new (g_str_hash, g_str_equal);
	camel_folder_set_lock_async (folder, TRUE);
}

/** End **/
