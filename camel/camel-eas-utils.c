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

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <eas-folder.h>
#include "camel-eas-utils.h"
#include "camel-eas-compat.h"

CamelFolderInfo *
camel_eas_utils_build_folder_info (CamelEasStore *store, const gchar *fid)
{
	CamelEasStoreSummary *eas_summary = store->summary;
	CamelFolderInfo *fi;
	gchar *url;

	url = camel_url_to_string (camel_service_get_camel_url(CAMEL_SERVICE (store)),
			(CAMEL_URL_HIDE_PASSWORD|
			 CAMEL_URL_HIDE_PARAMS|
			 CAMEL_URL_HIDE_AUTH) );

	if ( url[strlen (url) - 1] != '/') {
		gchar *temp_url;

		temp_url = g_strconcat (url, "/", NULL);
		g_free ((gchar *)url);
		url = temp_url;
	}

	fi = camel_folder_info_new ();
	fi->full_name = camel_eas_store_summary_get_folder_full_name (eas_summary,
								      fid, NULL);
#if EDS_CHECK_VERSION(3,1,0)
	fi->display_name = camel_eas_store_summary_get_folder_name (eas_summary,
								    fid, NULL);
#else
	fi->name = camel_eas_store_summary_get_folder_name (eas_summary,
							    fid, NULL);
	fi->uri = g_strconcat (url, fi->full_name, NULL);
#endif

	switch (camel_eas_store_summary_get_folder_type (eas_summary, fid, NULL)) {
	case EAS_FOLDER_TYPE_DEFAULT_INBOX:
		fi->flags = CAMEL_FOLDER_TYPE_INBOX;
		break;
	case EAS_FOLDER_TYPE_DEFAULT_OUTBOX:
		fi->flags = CAMEL_FOLDER_TYPE_OUTBOX;
		break;
	case EAS_FOLDER_TYPE_DEFAULT_DELETED_ITEMS:
		fi->flags = CAMEL_FOLDER_TYPE_TRASH;
		break;
	case EAS_FOLDER_TYPE_DEFAULT_SENT_ITEMS:
		fi->flags = CAMEL_FOLDER_TYPE_SENT;
		break;
	default:
		;
	}

	g_free (url);

	return fi;
}

#if 0
struct remove_esrc_data {
	gchar *fid;
	gchar *account_name;
	EasFolderType ftype;
};

static gboolean eas_do_remove_esource (gpointer user_data)
{
	struct remove_esrc_data *remove_data = user_data;


	eas_esource_utils_remove_esource (remove_data->fid,
					  remove_data->account_name,
					  remove_data->ftype);
	g_free (remove_data->fid);
	g_free (remove_data->account_name);
	g_free (remove_data);

	return FALSE;
}
#endif

static gboolean
eas_delete_folder_func (gpointer key, gpointer val, gpointer _store)
{
	CamelEasStore *eas_store = _store;
	CamelEasStoreSummary *eas_summary = eas_store->summary;
	CamelFolderInfo *fi;

	fi = camel_eas_utils_build_folder_info (eas_store, key);
	camel_store_folder_deleted (CAMEL_STORE (eas_store), fi);
	camel_eas_store_summary_remove_folder (eas_summary, key, NULL);
	return TRUE;
}

#if 0
static gboolean eas_utils_rename_folder (CamelEasStore *store, 
					 const gchar *fid, const gchar *changekey,
					 const gchar *pfid, const gchar *display_name,
					 const gchar *old_fname, GError **error)
{
	CamelEasStoreSummary *eas_summary = store->summary;
	CamelFolderInfo *fi;

	camel_eas_store_summary_set_change_key (eas_summary, fid, changekey);
	if (display_name)
		camel_eas_store_summary_set_folder_name (eas_summary, fid, display_name);
	if (pfid)
		camel_eas_store_summary_set_parent_folder_id (eas_summary, fid, pfid);

	if (ftype == EAS_FOLDER_TYPE_MAILBOX) {
		fi = camel_eas_utils_build_folder_info (store, fid);
		camel_store_folder_renamed ((CamelStore *) store, old_fname, fi);
	}

	return TRUE;
}
#endif
void
eas_utils_sync_folders (CamelEasStore *eas_store, GSList *folder_list)
{
	GError *error = NULL;
	GHashTable *old_hash;
	GSList *l;
	GSList* existing = camel_eas_store_summary_get_folders (eas_store->summary, NULL);

	old_hash = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
	for (l = existing; l; l = g_slist_next (l)) {
		EasFolder *folder = (EasFolder *) l->data;
		g_hash_table_insert (old_hash, folder, folder);
	}
	g_slist_free (existing);

	// iterate through existing list of folders
	for (l = folder_list; l != NULL; l = g_slist_next (l)) {
		EasFolder *folder = (EasFolder *) l->data;

		if (g_hash_table_remove (old_hash, folder->folder_id)) {
			/* We knew of a folder with this ID before */
			gchar *old_display_name, *old_parent_id;
			int old_type;
			
			old_display_name = camel_eas_store_summary_get_folder_name (eas_store->summary,
										    folder->folder_id, NULL);
			old_type = camel_eas_store_summary_get_folder_type (eas_store->summary,
									    folder->folder_id, NULL);
			old_parent_id = camel_eas_store_summary_get_parent_folder_id (eas_store->summary,
										      folder->folder_id, NULL);

			if (g_str_equal (folder->parent_id, old_parent_id) &&
			    g_str_equal (folder->display_name, old_display_name) &&
			    folder->type == old_type) {
				/* Complete match; no changes. Forget it */
				g_object_unref (folder);
			} else {
				gchar *old_full_name = camel_eas_store_summary_get_folder_full_name (eas_store->summary,
												     folder->folder_id, NULL);
				if (!g_str_equal (folder->display_name, old_display_name))
					camel_eas_store_summary_set_folder_name (eas_store->summary, folder->folder_id,
										  folder->display_name);
				if (!g_str_equal (folder->parent_id, old_parent_id))
					camel_eas_store_summary_set_parent_folder_id (eas_store->summary, folder->folder_id,
										      folder->parent_id);

				camel_eas_store_summary_set_folder_type (eas_store->summary, folder->folder_id, folder->type);

				if (eas_folder_type_is_mail (folder->type) ||
				    folder->type == EAS_FOLDER_TYPE_USER_CREATED_GENERIC) {
					CamelFolderInfo *fi = camel_eas_utils_build_folder_info (eas_store, folder->folder_id);
					camel_store_folder_renamed (CAMEL_STORE (eas_store), old_full_name, fi);
				}

				g_free (old_full_name);
				g_object_unref (folder);
			}
			g_free (old_display_name);
			g_free (old_parent_id);
		} else {
			/* No match in the hash table: this is a new folder */
			CamelFolderInfo *fi;

			camel_eas_store_summary_new_folder (eas_store->summary, folder);

			if (eas_folder_type_is_mail (folder->type) ||
			    folder->type == EAS_FOLDER_TYPE_USER_CREATED_GENERIC) {
				fi = camel_eas_utils_build_folder_info (eas_store, folder->folder_id);
				camel_store_folder_created ((CamelStore *) eas_store, fi);
			}

			g_object_unref (folder);
		}
	}
	/* We have eaten the objects; just free the list structure itself */
	g_slist_free (folder_list);

	/* Now, anything left in old_hash must be a folder that no longer exists */
	g_hash_table_foreach_remove (old_hash, eas_delete_folder_func, eas_store);
	g_hash_table_destroy (old_hash);

	camel_eas_store_summary_save (eas_store->summary, &error);
	if (error != NULL) {
		g_print ("Error while saving store summary %s \n", error->message);
		g_clear_error (&error);
	}
	return;
}

int
camel_eas_utils_sync_deleted_items (CamelEasFolder *eas_folder, GSList *items_deleted)
{
	CamelFolder *folder;
	const gchar *full_name;
	CamelFolderChangeInfo *ci;
	CamelEasStore *eas_store;
	GSList *l;
	int count = 0;
#if EDS_CHECK_VERSION(3,3,0)
	GList *uids_deleted = NULL;
#else
	GSList *uids_deleted = NULL;
#endif

	ci = camel_folder_change_info_new ();
	eas_store = (CamelEasStore *) camel_folder_get_parent_store ((CamelFolder *) eas_folder);

	folder = (CamelFolder *) eas_folder;
	full_name = camel_folder_get_full_name (folder);

	for (l = items_deleted; l != NULL; l = g_slist_next (l)) {
		EasEmailInfo *item = l->data;

#if ! EDS_CHECK_VERSION(3,3,0)
		camel_eas_summary_delete_id (folder->summary, item->server_id);
		uids_deleted = g_slist_prepend (uids_deleted, item->server_id);
#else
		uids_deleted = g_list_prepend (uids_deleted, item->server_id);
#endif
		camel_folder_change_info_remove_uid (ci, item->server_id);
		camel_data_cache_remove (eas_folder->cache, "cur", item->server_id, NULL);
		count++;
	}
	camel_db_delete_uids (((CamelStore *)eas_store)->cdb_w, full_name, uids_deleted, NULL);

	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);

	g_slist_foreach (items_deleted, (GFunc) g_object_unref, NULL);
	g_slist_free (items_deleted);
#if EDS_CHECK_VERSION(3,3,0)
	g_list_free (uids_deleted);
#else
	g_slist_free (uids_deleted);
#endif

	return count;
}

void
camel_eas_utils_clear_folder (CamelEasFolder *eas_folder)
{
	CamelFolder *folder = CAMEL_FOLDER (eas_folder);
	const gchar *full_name;
	CamelFolderChangeInfo *ci;
	CamelEasStore *eas_store;
	gchar *uid;
#if EDS_CHECK_VERSION(3,3,0)
	GList *uids_deleted = NULL;
	GPtrArray *known_uids = NULL;
	int i;
#else
	GSList *uids_deleted = NULL;
#endif

	if (!camel_folder_summary_count (folder->summary))
		return;

	ci = camel_folder_change_info_new ();
	eas_store = (CamelEasStore *) camel_folder_get_parent_store ((CamelFolder *) eas_folder);

	folder = (CamelFolder *) eas_folder;
	full_name = camel_folder_get_full_name (folder);

#if EDS_CHECK_VERSION(3,3,0)
	known_uids = camel_folder_summary_get_array (folder->summary);
	if (!known_uids)
		return;
	for (i = 0; i < known_uids->len; i++) {
		uid = g_ptr_array_index (known_uids, i);

		camel_folder_change_info_remove_uid (ci, uid);
		uids_deleted = g_list_prepend (uids_deleted, uid);
		camel_data_cache_remove (eas_folder->cache, "cur", uid, NULL);
	}
	camel_db_delete_uids (((CamelStore *)eas_store)->cdb_w, full_name, uids_deleted, NULL);
	g_list_free (uids_deleted);
#else
	while ( (uid = camel_folder_summary_uid_from_index (folder->summary, 0)) ) {
		camel_eas_summary_delete_id (folder->summary, uid);
		camel_folder_change_info_remove_uid (ci, uid);
		uids_deleted = g_slist_prepend (uids_deleted, uid);
		camel_data_cache_remove (eas_folder->cache, "cur", uid, NULL);
	}
	camel_db_delete_uids (((CamelStore *)eas_store)->cdb_w, full_name, uids_deleted, NULL);
	g_slist_foreach (uids_deleted, (GFunc) g_free, NULL);
	g_slist_free (uids_deleted);
#endif
	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);

}

#if 0
static gint
eas_utils_get_server_flags (EEasItem *item)
{
	gboolean flag;
	EasImportance importance;
	gint server_flags = 0;

	e_eas_item_is_read (item, &flag);
	if (flag)
		server_flags |= CAMEL_MESSAGE_SEEN;
	else
		server_flags &= ~CAMEL_MESSAGE_SEEN;

	e_eas_item_is_forwarded (item, &flag);
	if (flag)
		server_flags |= CAMEL_MESSAGE_FORWARDED;
	else
		server_flags &= ~CAMEL_MESSAGE_FORWARDED;

	e_eas_item_is_answered (item, &flag);
	if (flag)
		server_flags |= CAMEL_MESSAGE_ANSWERED;
	else
		server_flags &= ~CAMEL_MESSAGE_ANSWERED;

	importance = e_eas_item_get_importance (item);
	if (importance == EAS_ITEM_HIGH)
		server_flags |= CAMEL_MESSAGE_FLAGGED;

	/* TODO Update replied flags */

	return server_flags;
}

#endif

int
camel_eas_utils_sync_updated_items (CamelEasFolder *eas_folder, GSList *items_updated)
{
	CamelFolder *folder;
	CamelFolderChangeInfo *ci;
	GSList *l;
	int count = 0;

	ci = camel_folder_change_info_new ();
	folder = (CamelFolder *) eas_folder;

	for (l = items_updated; l != NULL; l = g_slist_next (l)) {
		EasEmailInfo *item = l->data;
		CamelEasMessageInfo *mi;

		mi = (CamelEasMessageInfo *) camel_folder_summary_get (folder->summary, item->server_id);
		if (mi) {
			gint flags = mi->info.flags;

			if (item->flags & EAS_VALID_READ) {
				if (item->flags & EAS_EMAIL_READ)
					flags |= CAMEL_MESSAGE_SEEN;
				else
					flags &= ~CAMEL_MESSAGE_SEEN;
			}
			if (item->flags & EAS_VALID_IMPORTANCE) {
				if (item->importance == EAS_IMPORTANCE_HIGH)
					flags |= CAMEL_MESSAGE_FLAGGED;
				else
					flags &= ~CAMEL_MESSAGE_FLAGGED;
			}
			if (camel_eas_update_message_info_flags (folder->summary, (CamelMessageInfo *)mi,
								 flags, NULL))
				camel_folder_change_info_change_uid (ci, mi->info.uid);

			mi->info.dirty = TRUE;

			camel_message_info_free (mi);
		}

		g_object_unref (item);
		count++;
	}

	camel_folder_summary_save_to_db (folder->summary, NULL);
	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);
	g_slist_free (items_updated);

	return count;
}

int
camel_eas_utils_sync_created_items (CamelEasFolder *eas_folder, GSList *items_created)
{
	CamelFolder *folder;
	CamelFolderChangeInfo *ci;
	GSList *l;
	int count = 0;

	if (!items_created)
		return 0;

	ci = camel_folder_change_info_new ();
	folder = (CamelFolder *) eas_folder;

	for (l = items_created; l != NULL; l = g_slist_next (l)) {
		EasEmailInfo *item = l->data;
		struct _camel_header_raw *camel_headers = NULL;
		CamelEasMessageInfo *mi;
		int flags = 0;
		GSList *hl;

		if (!item)
			continue;

		printf("Got item with Server ID %s, flags %u\n", item->server_id, item->flags);

		mi = (CamelEasMessageInfo *) camel_folder_summary_get (folder->summary, item->server_id);
		if (mi) {
			camel_message_info_free (mi);
			g_object_unref (item);
			continue;
		}

		for (hl = item->headers; hl; hl = g_slist_next(hl)) {
			EasEmailHeader *hdr = hl->data;

			camel_header_raw_append (&camel_headers, hdr->name, hdr->value, 0);
		}

		mi = (CamelEasMessageInfo *)camel_folder_summary_info_new_from_header (folder->summary, camel_headers);
		if (mi->info.content == NULL) {
			//mi->info.content = camel_folder_summary_content_info_new_from_header (folder->summary, camel_headers);
			mi->info.content = camel_folder_summary_content_info_new (folder->summary);
			mi->info.content->type = camel_content_type_new ("multipart", "mixed");
		}

		camel_header_raw_clear (&camel_headers);

		mi->info.uid = camel_pstring_strdup (item->server_id);
		mi->info.size = item->estimated_size;
		mi->info.date_received = item->date_received;

		if (item->attachments)
			flags |= CAMEL_MESSAGE_ATTACHMENTS;

		if (item->flags & EAS_EMAIL_READ)
			flags |= CAMEL_MESSAGE_SEEN;
		if (item->flags & EAS_EMAIL_ANSWERED_LAST)
			flags |= CAMEL_MESSAGE_ANSWERED;
		if (item->flags & EAS_EMAIL_FORWARDED_LAST)
			flags |= CAMEL_MESSAGE_FORWARDED;
		if (item->importance == EAS_IMPORTANCE_HIGH)
			flags |= CAMEL_MESSAGE_FLAGGED;

		camel_eas_summary_add_message_info (folder->summary, flags,
						    (CamelMessageInfo *) mi);
		camel_folder_change_info_add_uid (ci, item->server_id);
		camel_folder_change_info_recent_uid (ci, item->server_id);

		g_object_unref (item);

		count++;
	}

	camel_folder_summary_save_to_db (folder->summary, NULL);
	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);
	g_slist_free (items_created);

	return count;
}

#if 0

struct _create_mime_msg_data {
	CamelMimeMessage *message;
	gint32 message_camel_flags;
	CamelAddress *from;
};

static void
create_mime_message_cb (ESoapMessage *msg, gpointer user_data)
{
	struct _create_mime_msg_data *create_data = user_data;
	CamelStream *mem, *filtered;
	CamelMimeFilter *filter;
	GByteArray *bytes;
	gchar *base64;

	e_soap_message_start_element (msg, "Message", NULL, NULL);
	e_soap_message_start_element (msg, "MimeContent", NULL, NULL);

	/* This is horrid. We really need to extend ESoapMessage to allow us
	   to stream this directly rather than storing it in RAM. Which right
	   now we are doing about four times: the GByteArray in the mem stream,
	   then the base64 version, then the xmlDoc, then the soup request. */
	camel_mime_message_set_best_encoding (create_data->message,
					      CAMEL_BESTENC_GET_ENCODING,
					      CAMEL_BESTENC_8BIT);

	mem = camel_stream_mem_new();
	filtered = camel_stream_filter_new (mem);

	filter = camel_mime_filter_crlf_new (CAMEL_MIME_FILTER_CRLF_ENCODE,
				     CAMEL_MIME_FILTER_CRLF_MODE_CRLF_ONLY);
	camel_stream_filter_add (CAMEL_STREAM_FILTER (filtered), filter);
	g_object_unref (filter);

	EVO3_sync(camel_data_wrapper_write_to_stream)
				(CAMEL_DATA_WRAPPER (create_data->message),
				 filtered, EVO3(NULL,) NULL);
	camel_stream_flush (filtered, EVO3(NULL,) NULL);
	camel_stream_flush (mem, EVO3(NULL,) NULL);
	bytes = camel_stream_mem_get_byte_array (CAMEL_STREAM_MEM (mem));

	base64 = g_base64_encode (bytes->data, bytes->len);
	g_object_unref (mem);
	g_object_unref (filtered);

	e_soap_message_write_string (msg, base64);
	g_free (base64);

	e_soap_message_end_element (msg); /* MimeContent */

	/* FIXME: Handle From address and message_camel_flags */

	e_soap_message_end_element (msg); /* Message */

	g_free (create_data);
}

gboolean
camel_eas_utils_create_mime_message (EEasConnection *cnc, const gchar *disposition,
				     const gchar *save_folder, CamelMimeMessage *message,
				     gint32 message_camel_flags, CamelAddress *from,
				     gchar **itemid, gchar **changekey,
				     GCancellable *cancellable, GError **error)
{
	struct _create_mime_msg_data *create_data;
	GSList *ids;
	EEasItem *item;
	const EasId *easid;
	gboolean res;

	create_data = g_new0 (struct _create_mime_msg_data, 1);

	create_data->message = message;
	create_data->message_camel_flags = message_camel_flags;
	create_data->from = from;

	res = e_eas_connection_create_items (cnc, EAS_PRIORITY_MEDIUM,
					     disposition, NULL, save_folder,
					     create_mime_message_cb, create_data,
					     &ids, cancellable, error);
	if (!res || (!itemid && !changekey))
		return res;

	item = (EEasItem *)ids->data;
	if (!item || !(easid = e_eas_item_get_id (item))) {
		g_set_error(error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			    _("CreateItem call failed to return ID for new message"));
		return FALSE;
	}

	if (itemid)
		*itemid = g_strdup (easid->id);
	if (changekey)
		*changekey = g_strdup (easid->change_key);

	g_object_unref (item);
	g_slist_free (ids);
	return TRUE;
}
#endif
