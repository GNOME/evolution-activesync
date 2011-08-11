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

#include "../libeasmail/src/eas-folder.h"
#include "camel-eas-utils.h"
#include "camel-eas-compat.h"

CamelFolderInfo *
camel_eas_utils_build_folder_info (CamelEasStore *store, const gchar *fid)
{
	CamelEasStoreSummary *eas_summary = store->summary;
	CamelFolderInfo *fi;
	gchar *url;

	url = camel_url_to_string (CAMEL_SERVICE (store)->url,
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
	fi->name = camel_eas_store_summary_get_folder_name (eas_summary,
							    fid, NULL);
	fi->uri = g_strconcat (url, fi->full_name, NULL);
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
static void
sync_updated_folders (CamelEasStore *store, GSList *updated_folders)
{
#if 0
	CamelEasStoreSummary *eas_summary = store->summary;
	GSList *l;

	for (l = updated_folders; l != NULL; l = g_slist_next (l)) {
		EEasFolder *eas_folder = (EEasFolder *)	l->data;
		EasFolderType ftype;
		gchar *folder_name;
		gchar *display_name;
		const EasFolderId *fid, *pfid;

		ftype = e_eas_folder_get_folder_type (eas_folder);
		if (ftype == EAS_FOLDER_TYPE_CALENDAR ||
		    ftype == EAS_FOLDER_TYPE_TASKS ||
		    ftype == EAS_FOLDER_TYPE_CONTACTS) {
			/* TODO Update esource */
		} else 	if (ftype != EAS_FOLDER_TYPE_MAILBOX)
			continue;

		fid = e_eas_folder_get_id (eas_folder);
		folder_name = camel_eas_store_summary_get_folder_full_name (eas_summary, fid->id, NULL);

		pfid = e_eas_folder_get_parent_id (eas_folder);
		display_name = g_strdup (e_eas_folder_get_name (eas_folder));

		/* If the folder is moved or renamed (which are separate
		   operations in Exchange, unfortunately, then the name
		   or parent folder will change. Handle both... */
		if (pfid || display_name) {
			GError *error = NULL;
			gchar *new_fname = NULL;

			if (pfid) {
				gchar *pfname;

				/* If the display name wasn't changed, its basename is still
				   the same as it was before... */
				if (!display_name)
					display_name = camel_eas_store_summary_get_folder_name (eas_summary,
										fid->id, NULL);
				if (!display_name)
					goto done;

				pfname = camel_eas_store_summary_get_folder_full_name (eas_summary, pfid->id, NULL);

				/* If the lookup failed, it'll be because the new parent folder
				   is the message folder root. */
				if (pfname) {
					new_fname = g_strconcat (pfname, "/", display_name, NULL);
					g_free (pfname);
				} else
					new_fname = g_strdup (display_name);
			} else {
				/* Parent folder not changed; just basename */
				const gchar *last_slash;

				/* Append new display_name to old parent directory name... */
				last_slash = g_strrstr (folder_name, "/");
				if (last_slash)
					new_fname = g_strdup_printf ("%.*s/%s", (int)(last_slash - folder_name),
								     folder_name, display_name);
				else /* ...unless it was a child of the root folder */
					new_fname = g_strdup (display_name);
			}

			if (strcmp(new_fname, folder_name))
				eas_utils_rename_folder (store, ftype,
							 fid->id, fid->change_key,
							 pfid?pfid->id:NULL,
							 display_name, folder_name, &error);
			g_free (new_fname);
			g_clear_error (&error);
		}
 done:
		g_free (folder_name);
		g_free (display_name);
	}
#endif
}

static void
sync_created_folders (CamelEasStore *eas_store, GSList *created_folders)
{
	GSList *l;

	for (l = created_folders; l != NULL; l = g_slist_next (l)) {
		EasFolder *folder = (EasFolder *) l->data;
		CamelFolderInfo *fi;

		camel_eas_store_summary_new_folder (eas_store->summary, folder);

		if (eas_folder_type_is_mail (folder->type) ||
		    folder->type == EAS_FOLDER_TYPE_USER_CREATED_GENERIC) {
			fi = camel_eas_utils_build_folder_info (eas_store, folder->folder_id);
			camel_store_folder_created ((CamelStore *) eas_store, fi);
		}
		g_object_unref (folder);
	}
	g_slist_free (created_folders);
}

void
eas_utils_sync_folders (CamelEasStore *eas_store, GSList *folder_list)
{
	GError *error = NULL;
	GHashTable *old_hash;
	GSList *l;
	GSList* created_folders = NULL;
	GSList* updated_folders = NULL;
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
				/* The folder has been changed */
				updated_folders = g_slist_append (updated_folders, folder);
			}
			g_free (old_display_name);
			g_free (old_parent_id);
		} else {
			/* No match in the hash table: this is a new folder */
			created_folders = g_slist_append (created_folders, folder);
		}
	}
	/* We have eaten the objects; just free the list structure itself */
	g_slist_free (folder_list);

	/* Now, anything left in old_hash must be a folder that no longer exists */
	g_hash_table_foreach_remove (old_hash, eas_delete_folder_func, eas_store);
	g_hash_table_destroy (old_hash);

	sync_updated_folders (eas_store, updated_folders);
	sync_created_folders (eas_store, created_folders);

	camel_eas_store_summary_save (eas_store->summary, &error);
	if (error != NULL) {
		g_print ("Error while saving store summary %s \n", error->message);
		g_clear_error (&error);
	}
	return;
}

void
camel_eas_utils_sync_deleted_items (CamelEasFolder *eas_folder, GSList *items_deleted)
{
	CamelFolder *folder;
	const gchar *full_name;
	GSList *uids_deleted = NULL;
	CamelFolderChangeInfo *ci;
	CamelEasStore *eas_store;
	GSList *l;

	ci = camel_folder_change_info_new ();
	eas_store = (CamelEasStore *) camel_folder_get_parent_store ((CamelFolder *) eas_folder);

	folder = (CamelFolder *) eas_folder;
	full_name = camel_folder_get_full_name (folder);

	for (l = items_deleted; l != NULL; l = g_slist_next (l)) {
		EasEmailInfo *item = l->data;

		camel_eas_summary_delete_id (folder->summary, item->server_id);
		camel_folder_change_info_remove_uid (ci, item->server_id);
		uids_deleted = g_slist_prepend (uids_deleted, item->server_id);
		camel_data_cache_remove (eas_folder->cache, "cur", item->server_id, NULL);
	}
	camel_db_delete_uids (((CamelStore *)eas_store)->cdb_w, full_name, uids_deleted, NULL);

	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);

	g_slist_foreach (items_deleted, (GFunc) g_object_unref, NULL);
	g_slist_free (items_deleted);
	g_slist_free (uids_deleted);
}

void
camel_eas_utils_clear_folder (CamelEasFolder *eas_folder)
{
	CamelFolder *folder = CAMEL_FOLDER (eas_folder);
	const gchar *full_name;
	GSList *uids_deleted = NULL;
	CamelFolderChangeInfo *ci;
	CamelEasStore *eas_store;
	gchar *uid;

	if (!camel_folder_summary_count (folder->summary))
		return;

	ci = camel_folder_change_info_new ();
	eas_store = (CamelEasStore *) camel_folder_get_parent_store ((CamelFolder *) eas_folder);

	folder = (CamelFolder *) eas_folder;
	full_name = camel_folder_get_full_name (folder);

	while ( (uid = camel_folder_summary_uid_from_index (folder->summary, 0)) ) {
		camel_eas_summary_delete_id (folder->summary, uid);
		camel_folder_change_info_remove_uid (ci, uid);
		uids_deleted = g_slist_prepend (uids_deleted, uid);
		camel_data_cache_remove (eas_folder->cache, "cur", uid, NULL);
	}
	camel_db_delete_uids (((CamelStore *)eas_store)->cdb_w, full_name, uids_deleted, NULL);

	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);

	g_slist_foreach (uids_deleted, (GFunc) g_free, NULL);
	g_slist_free (uids_deleted);
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

void
camel_eas_utils_sync_updated_items (CamelEasFolder *eas_folder, GSList *items_updated)
{
	CamelFolder *folder;
	CamelFolderChangeInfo *ci;
	GSList *l;

	ci = camel_folder_change_info_new ();
	folder = (CamelFolder *) eas_folder;

	for (l = items_updated; l != NULL; l = g_slist_next (l)) {
		EasEmailInfo *item = l->data;
		CamelEasMessageInfo *mi;

		mi = (CamelEasMessageInfo *) camel_folder_summary_uid (folder->summary, item->server_id);
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
	}

	camel_folder_summary_save_to_db (folder->summary, NULL);
	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);
	g_slist_free (items_updated);
}

void
camel_eas_utils_sync_created_items (CamelEasFolder *eas_folder, GSList *items_created)
{
	CamelFolder *folder;
	CamelFolderChangeInfo *ci;
	GSList *l;

	if (!items_created)
		return;

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

		mi = (CamelEasMessageInfo *) camel_folder_summary_uid (folder->summary, item->server_id);
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
	}

	camel_folder_summary_save_to_db (folder->summary, NULL);
	camel_folder_changed ((CamelFolder *) eas_folder, ci);
	camel_folder_change_info_free (ci);
	g_slist_free (items_created);
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
