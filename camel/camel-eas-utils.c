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
                               (CAMEL_URL_HIDE_PASSWORD |
                                CAMEL_URL_HIDE_PARAMS |
                                CAMEL_URL_HIDE_AUTH));

    if (url[strlen (url) - 1] != '/')
    {
        gchar *temp_url;

        temp_url = g_strconcat (url, "/", NULL);
        g_free ( (gchar *) url);
        url = temp_url;
    }

    fi = camel_folder_info_new ();
    fi->full_name = camel_eas_store_summary_get_folder_full_name (eas_summary,
                    fid, NULL);
    fi->name = camel_eas_store_summary_get_folder_name (eas_summary,
                                                        fid, NULL);
    fi->uri = g_strconcat (url, fi->full_name, NULL);
    switch (camel_eas_store_summary_get_folder_type (eas_summary, fid, NULL))
    {
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
    fi->unread = camel_eas_store_summary_get_folder_unread (eas_summary,
                                                            fid, NULL);
    fi->total = camel_eas_store_summary_get_folder_total (eas_summary,
                                                          fid, NULL);

    g_free (url);

    return fi;
}

#if 0
struct remove_esrc_data
{
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

static void
sync_deleted_folders (CamelEasStore *store, GSList *deleted_folders)
{
    CamelEasStoreSummary *eas_summary = store->summary;
    GSList *l;

    for (l = deleted_folders; l != NULL; l = g_slist_next (l))
    {
        const gchar *fid = l->data;
        CamelFolderInfo *fi;
        GError *error = NULL;

        if (!camel_eas_store_summary_has_folder (eas_summary, fid))
            continue;

        camel_eas_store_summary_remove_folder (eas_summary, fid, NULL);
#if 0
        ftype = camel_eas_store_summary_get_folder_type (eas_summary, fid, NULL);
        if (ftype == EAS_FOLDER_TYPE_MAILBOX)
        {
            fi = camel_eas_utils_build_folder_info (store, fid);

            camel_store_folder_deleted ( (CamelStore *) store, fi);
        }
#endif
    }
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

    if (ftype == EAS_FOLDER_TYPE_MAILBOX)
    {
        fi = camel_eas_utils_build_folder_info (store, fid);
        camel_store_folder_renamed ( (CamelStore *) store, old_fname, fi);
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

    for (l = updated_folders; l != NULL; l = g_slist_next (l))
    {
        EEasFolder *eas_folder = (EEasFolder *) l->data;
        EasFolderType ftype;
        gchar *folder_name;
        gchar *display_name;
        const EasFolderId *fid, *pfid;

        ftype = e_eas_folder_get_folder_type (eas_folder);
        if (ftype == EAS_FOLDER_TYPE_CALENDAR ||
                ftype == EAS_FOLDER_TYPE_TASKS ||
                ftype == EAS_FOLDER_TYPE_CONTACTS)
        {
            /* TODO Update esource */
        }
        else  if (ftype != EAS_FOLDER_TYPE_MAILBOX)
            continue;

        fid = e_eas_folder_get_id (eas_folder);
        folder_name = camel_eas_store_summary_get_folder_full_name (eas_summary, fid->id, NULL);

        pfid = e_eas_folder_get_parent_id (eas_folder);
        display_name = g_strdup (e_eas_folder_get_name (eas_folder));

        /* If the folder is moved or renamed (which are separate
           operations in Exchange, unfortunately, then the name
           or parent folder will change. Handle both... */
        if (pfid || display_name)
        {
            GError *error = NULL;
            gchar *new_fname = NULL;

            if (pfid)
            {
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
                if (pfname)
                {
                    new_fname = g_strconcat (pfname, "/", display_name, NULL);
                    g_free (pfname);
                }
                else
                    new_fname = g_strdup (display_name);
            }
            else
            {
                /* Parent folder not changed; just basename */
                const gchar *last_slash;

                /* Append new display_name to old parent directory name... */
                last_slash = g_strrstr (folder_name, "/");
                if (last_slash)
                    new_fname = g_strdup_printf ("%.*s/%s", (int) (last_slash - folder_name),
                                                 folder_name, display_name);
                else /* ...unless it was a child of the root folder */
                    new_fname = g_strdup (display_name);
            }

            if (strcmp (new_fname, folder_name))
                eas_utils_rename_folder (store, ftype,
                                         fid->id, fid->change_key,
                                         pfid ? pfid->id : NULL,
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

    for (l = created_folders; l != NULL; l = g_slist_next (l))
    {
        EasFolder *folder = (EasFolder *) l->data;
        CamelFolderInfo *fi;

        camel_eas_store_summary_new_folder (eas_store->summary, folder);

        if (eas_folder_type_is_mail (folder->type))
        {
            fi = camel_eas_utils_build_folder_info (eas_store, folder->folder_id);
            camel_store_folder_created ( (CamelStore *) eas_store, fi);
        }
    }
}

void
eas_utils_sync_folders (CamelEasStore *eas_store, GSList *created_folders, GSList *deleted_folders, GSList *updated_folders)
{
    GError *error = NULL;

    sync_deleted_folders (eas_store, deleted_folders);
    sync_updated_folders (eas_store, updated_folders);
    sync_created_folders (eas_store, created_folders);

    camel_eas_store_summary_save (eas_store->summary, &error);
    if (error != NULL)
    {
        g_print ("Error while saving store summary %s \n", error->message);
        g_clear_error (&error);
    }
    return;
}

void
camel_eas_utils_sync_deleted_items (CamelEasFolder *eas_folder, GSList *items_deleted)
{
#if 0

    CamelFolder *folder;
    const gchar *full_name;
    CamelFolderChangeInfo *ci;
    CamelEasStore *eas_store;
    GSList *l;

    ci = camel_folder_change_info_new ();
    eas_store = (CamelEasStore *) camel_folder_get_parent_store ( (CamelFolder *) eas_folder);

    folder = (CamelFolder *) eas_folder;
    full_name = camel_folder_get_full_name (folder);

    for (l = items_deleted; l != NULL; l = g_slist_next (l))
    {
        gchar *id = (gchar *) l->data;

        camel_eas_summary_delete_id (folder->summary, id);
        camel_folder_change_info_remove_uid (ci, id);
    }
    camel_db_delete_uids ( ( (CamelStore *) eas_store)->cdb_w, full_name, items_deleted, NULL);

    camel_folder_changed ( (CamelFolder *) eas_folder, ci);
    camel_folder_change_info_free (ci);

    g_slist_foreach (items_deleted, (GFunc) g_free, NULL);
    g_slist_free (items_deleted);
#endif
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

static const gchar *
form_email_string_from_mb (const EasMailbox *mb)
{
    const gchar *ret = NULL;

    if (mb)
    {
        GString *str;

        str = g_string_new ("");
        if (mb->name && mb->name[0])
        {
            str = g_string_append (str, mb->name);
            str = g_string_append (str, " ");
        }
        else
        {
            str = g_string_append (str, mb->email);
            str = g_string_append (str, " ");
        }

        g_string_append (str, "<");
        str = g_string_append (str, mb->email);
        g_string_append (str, ">");

        ret = camel_pstring_add (str->str, TRUE);
        g_string_free (str, FALSE);

        return ret;
    }
    else
        return camel_pstring_strdup ("");
}

static const gchar *
form_recipient_list (const GSList *recipients)
{
    const GSList *l;
    GString *str = NULL;
    const gchar *ret;

    if (!recipients)
        return NULL;

    for (l = recipients; l != NULL; l = g_slist_next (l))
    {
        EasMailbox *mb = (EasMailbox *) l->data;
        const gchar *mb_str = form_email_string_from_mb (mb);

        if (!str)
            str = g_string_new ("");
        else
            str = g_string_append (str, ", ");

        str = g_string_append (str, mb_str);
    }

    ret = camel_pstring_add (str->str, TRUE);
    g_string_free (str, FALSE);

    return ret;
}

static guint8 *
get_md5_digest (const guchar *str)
{
    guint8 *digest;
    gsize length;
    GChecksum *checksum;

    length = g_checksum_type_get_length (G_CHECKSUM_MD5);
    digest = g_malloc0 (length);

    checksum = g_checksum_new (G_CHECKSUM_MD5);
    g_checksum_update (checksum, str, -1);
    g_checksum_get_digest (checksum, digest, &length);
    g_checksum_free (checksum);

    return digest;
}

static void
eas_set_threading_data (CamelEasMessageInfo *mi, EEasItem *item)
{
    const gchar *references, *inreplyto;
    gint count = 0;
    const gchar *message_id;
    struct _camel_header_references *refs, *irt, *scan;
    guint8 *digest;
    gchar *msgid;

    /* set message id */
    message_id = e_eas_item_get_msg_id (item);
    msgid = camel_header_msgid_decode (message_id);
    if (msgid)
    {
        digest = get_md5_digest ( (const guchar *) msgid);
        memcpy (mi->info.message_id.id.hash, digest, sizeof (mi->info.message_id.id.hash));
        g_free (digest);
        g_free (msgid);
    }

    /* Process References: header */
    references = e_eas_item_get_references (item);
    refs = camel_header_references_decode (references);

    /* Prepend In-Reply-To: contents to References: for summary info */
    inreplyto = e_eas_item_get_in_replyto (item);
    irt = camel_header_references_inreplyto_decode (inreplyto);
    if (irt)
    {
        irt->next = refs;
        refs = irt;
    }
    if (!refs)
        return;

    count = camel_header_references_list_size (&refs);
    mi->info.references = g_malloc (sizeof (*mi->info.references) + ( (count - 1) * sizeof (mi->info.references->references[0])));
    scan = refs;
    count = 0;

    while (scan)
    {
        digest = get_md5_digest ( (const guchar *) scan->id);
        memcpy (mi->info.references->references[count].id.hash, digest, sizeof (mi->info.message_id.id.hash));
        g_free (digest);

        count++;
        scan = scan->next;
    }

    mi->info.references->size = count;
    camel_header_references_list_clear (&refs);
}
#endif

void
camel_eas_utils_sync_updated_items (CamelEasFolder *eas_folder, GSList *items_updated)
{
#if 0
    CamelFolder *folder;
    CamelFolderChangeInfo *ci;
    GSList *l;

    ci = camel_folder_change_info_new ();
    folder = (CamelFolder *) eas_folder;

    for (l = items_updated; l != NULL; l = g_slist_next (l))
    {
        EEasItem *item = (EEasItem *) l->data;
        const EasId *id;
        CamelEasMessageInfo *mi;

        id = e_eas_item_get_id (item);
        mi = (CamelEasMessageInfo *) camel_folder_summary_uid (folder->summary, id->id);
        if (mi)
        {
            gint server_flags;

            server_flags = eas_utils_get_server_flags (item);
            if (camel_eas_update_message_info_flags (folder->summary, (CamelMessageInfo *) mi,
                                                     server_flags, NULL))
                camel_folder_change_info_change_uid (ci, mi->info.uid);

            mi->change_key = g_strdup (id->change_key);
            mi->info.dirty = TRUE;

            camel_message_info_free (mi);
            g_object_unref (item);
            continue;
        }

        g_object_unref (item);
    }

    camel_folder_summary_save_to_db (folder->summary, NULL);
    camel_folder_changed ( (CamelFolder *) eas_folder, ci);
    camel_folder_change_info_free (ci);
    g_slist_free (items_updated);
#endif
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

    for (l = items_created; l != NULL; l = g_slist_next (l))
    {
        EasEmailInfo *item = l->data;
        CamelEasMessageInfo *mi;
        GSList *hl;

        if (!item)
            continue;

        printf ("Got item with Server ID %s, flags %u\n", item->server_id, item->flags);

        mi = (CamelEasMessageInfo *) camel_folder_summary_uid (folder->summary, item->server_id);
        if (mi)
        {
            camel_message_info_free (mi);
            g_object_unref (item);
            continue;
        }

        mi = (CamelEasMessageInfo *) camel_message_info_new (folder->summary);

        for (hl = item->headers; hl; hl = g_slist_next (hl))
        {
            EasEmailHeader *hdr = hl->data;

            printf (" %s: %s\n", hdr->name, hdr->value);

            if (!g_ascii_strcasecmp (hdr->name, "Subject"))
                mi->info.subject = camel_pstring_strdup (hdr->value);
            else if (!g_ascii_strcasecmp (hdr->name, "From"))
                mi->info.from = camel_pstring_strdup (hdr->value);
            else if (!g_ascii_strcasecmp (hdr->name, "Cc"))
                mi->info.cc = camel_pstring_strdup (hdr->value);
            else if (!g_ascii_strcasecmp (hdr->name, "To"))
                mi->info.to = camel_pstring_strdup (hdr->value);
            else if (!g_ascii_strcasecmp (hdr->name, "Received"))
                mi->info.date_received = time (NULL);
        }

        if (mi->info.content == NULL)
        {
            mi->info.content = camel_folder_summary_content_info_new (folder->summary);
            mi->info.content->type = camel_content_type_new ("multipart", "mixed");
        }

        mi->info.uid = camel_pstring_strdup (item->server_id);
        //      mi->info.size = e_eas_item_get_size (item);

        //      mi->info.date_sent = e_eas_item_get_date_sent (item);
        //      mi->info.date_received = e_eas_item_get_date_received (item);


        if (item->attachments)
            mi->info.flags |= CAMEL_MESSAGE_ATTACHMENTS;

        //  eas_set_threading_data (mi, item);
        //server_flags = eas_utils_get_server_flags (item);

        camel_eas_summary_add_message_info (folder->summary, 0,//server_flags,
                                            (CamelMessageInfo *) mi);
        camel_folder_change_info_add_uid (ci, item->server_id);
        camel_folder_change_info_recent_uid (ci, item->server_id);

        g_object_unref (item);
    }

    camel_folder_summary_save_to_db (folder->summary, NULL);
    camel_folder_changed ( (CamelFolder *) eas_folder, ci);
    camel_folder_change_info_free (ci);
    g_slist_free (items_created);
}

#if 0

struct _create_mime_msg_data
{
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

    EVO3_sync (camel_data_wrapper_write_to_stream)
    (CAMEL_DATA_WRAPPER (create_data->message),
     filtered, EVO3 (NULL,) NULL);
    camel_stream_flush (filtered, EVO3 (NULL,) NULL);
    camel_stream_flush (mem, EVO3 (NULL,) NULL);
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

    item = (EEasItem *) ids->data;
    if (!item || ! (easid = e_eas_item_get_id (item)))
    {
        g_set_error (error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
                     _ ("CreateItem call failed to return ID for new message"));
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
