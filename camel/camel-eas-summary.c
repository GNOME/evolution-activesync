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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "camel-eas-folder.h"
#include "camel-eas-summary.h"

#define CAMEL_EAS_SUMMARY_VERSION (1)

#define EXTRACT_FIRST_DIGIT(val) part ? val=strtoul (part, &part, 10) : 0;
#define EXTRACT_DIGIT(val) part++; part ? val=strtoul (part, &part, 10) : 0;

#define d(x)

/*Prototypes*/
#define SUM_DB_RETTYPE gboolean
#define SUM_DB_RET_OK TRUE
#define SUM_DB_RET_ERR FALSE
static gboolean summary_header_load (CamelFolderSummary *s, CamelStoreDBFolderRecord *record);
static gboolean summary_header_save (CamelFolderSummary *s, CamelStoreDBFolderRecord *inout_record, GError **error);

/*End of Prototypes*/

G_DEFINE_TYPE (CamelEasSummary, camel_eas_summary, CAMEL_TYPE_FOLDER_SUMMARY)

static void
eas_summary_finalize (GObject *object)
{
	CamelEasSummary *eas_summary = CAMEL_EAS_SUMMARY (object);

	g_free (eas_summary->sync_state);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (camel_eas_summary_parent_class)->finalize (object);
}

static void
camel_eas_summary_class_init (CamelEasSummaryClass *class)
{
	CamelFolderSummaryClass *folder_summary_class;
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (class);
	object_class->finalize = eas_summary_finalize;

	folder_summary_class = CAMEL_FOLDER_SUMMARY_CLASS (class);
	folder_summary_class->message_info_type = CAMEL_TYPE_EAS_MESSAGE_INFO;
	folder_summary_class->summary_header_save = summary_header_save;
	folder_summary_class->summary_header_load = summary_header_load;
}

static void
camel_eas_summary_init (CamelEasSummary *eas_summary)
{
}

/**
 * camel_eas_summary_new:
 * @filename: the file to store the summary in.
 *
 * This will create a new CamelEasSummary object and read in the
 * summary data from disk, if it exists.
 *
 * Returns: A new CamelEasSummary object.
 **/
CamelFolderSummary *
camel_eas_summary_new (struct _CamelFolder *folder, const gchar *filename)
{
	CamelFolderSummary *summary;

	summary = g_object_new (CAMEL_TYPE_EAS_SUMMARY,
				"folder", folder, NULL);

	camel_folder_summary_load (summary, NULL);

	return summary;
}

static gboolean
summary_header_load (CamelFolderSummary *s, CamelStoreDBFolderRecord *record)
{
	CamelEasSummary *gms = CAMEL_EAS_SUMMARY (s);
	gchar *part;

	if (!CAMEL_FOLDER_SUMMARY_CLASS (camel_eas_summary_parent_class)->summary_header_load (s, record))
		return FALSE;

	part = record->bdata;

	if (part)
		EXTRACT_FIRST_DIGIT(gms->version);

	if (part && part++ && strcmp (part, "(null)"))
		gms->sync_state = g_strndup (part, 64);
	else
		gms->sync_state = g_strndup ("0", 64);

	return TRUE;
}

static gboolean
summary_header_save (CamelFolderSummary *s, CamelStoreDBFolderRecord *inout_record, GError **error)
{
	CamelEasSummary *ims = CAMEL_EAS_SUMMARY (s);

	if (!CAMEL_FOLDER_SUMMARY_CLASS (camel_eas_summary_parent_class)->summary_header_save (s, inout_record, error))
		return FALSE;

	g_free (inout_record->bdata);
	inout_record->bdata = g_strdup_printf ("%d %s", CAMEL_EAS_SUMMARY_VERSION, ims->sync_state);

	return TRUE;
}

void
camel_eas_summary_add_message	(CamelFolderSummary *summary,
				 const gchar *uid,
				 CamelMimeMessage *message)
{
	CamelMessageInfo *mi;
	CamelMessageInfo *info;

	info = camel_folder_summary_get (summary, uid);

	/* Create summary entry */
	mi = camel_folder_summary_info_new_from_message (summary, message);

	camel_message_info_set_abort_notifications (mi, TRUE);

	/* Copy flags 'n' tags */
	camel_message_info_set_flags (mi, ~0, camel_message_info_get_flags (info));
	camel_message_info_take_user_flags (mi, camel_message_info_dup_user_flags (info));
	camel_message_info_take_user_tags (mi, camel_message_info_dup_user_tags (info));
	camel_message_info_set_size (mi, camel_message_info_get_size (info));
	camel_message_info_set_uid (mi, uid);

	camel_message_info_set_abort_notifications (mi, FALSE);
	camel_folder_summary_add (summary, mi, FALSE);
	g_clear_object (&info);
}

void
camel_eas_summary_add_message_info	(CamelFolderSummary *summary,
					 guint32 server_flags,
					 CamelMessageInfo *mi)
{
	camel_message_info_set_flags (mi, server_flags, server_flags);
	camel_eas_message_info_set_server_flags (CAMEL_EAS_MESSAGE_INFO (mi), server_flags);
	camel_message_info_set_folder_flagged (mi, FALSE);

	camel_folder_summary_add (summary, mi, FALSE);
}

static gboolean
eas_update_user_flags (CamelMessageInfo *info,
		       const CamelNamedFlags *server_user_flags)
{
	gboolean changed = FALSE;
	gboolean set_cal = FALSE;

	if (camel_message_info_get_user_flag (info, "$has_cal"))
		set_cal = TRUE;

	changed = camel_message_info_take_user_flags (info, camel_named_flags_copy (server_user_flags));

	/* reset the calendar flag if it was set in messageinfo before */
	if (set_cal)
		camel_message_info_set_user_flag (info, "$has_cal", TRUE);

	return changed;
}

gboolean
camel_eas_update_message_info_flags	(CamelFolderSummary *summary,
					 CamelMessageInfo *info,
					 guint32 server_flags,
					 const CamelNamedFlags *server_user_flags)
{
	CamelEasMessageInfo *einfo = (CamelEasMessageInfo *) info;
	gboolean changed = FALSE;

	if (server_flags != camel_eas_message_info_get_server_flags (einfo)) {
		guint32 server_set, server_cleared, has_stored;

		has_stored = camel_eas_message_info_get_server_flags (einfo);
		server_set = server_flags & ~has_stored;
		server_cleared = has_stored & ~server_flags;

		camel_message_info_set_flags (info, server_set | server_cleared,
					      (camel_message_info_get_flags (info) | server_set) & ~server_cleared);
                camel_eas_message_info_set_server_flags (einfo, server_flags);
		changed = TRUE;
	}

	/* TODO test user_flags after enabling it */
	if (server_user_flags && eas_update_user_flags (info, server_user_flags))
		changed = TRUE;

	return changed;
}

void
eas_summary_clear	(CamelFolderSummary *summary,
			 gboolean uncache)
{
	CamelFolderChangeInfo *changes;
	GHashTable *hash;
	GHashTableIter iter;
	gpointer key;

	changes = camel_folder_change_info_new ();
	hash = camel_folder_summary_get_hash (summary);
	g_hash_table_iter_init (&iter, hash);
	while (g_hash_table_iter_next (&iter, &key, NULL)) {
		const gchar *uid = key;
		camel_folder_change_info_remove_uid (changes, uid);
		camel_folder_summary_remove_uid (summary, uid);
	}
	g_hash_table_unref (hash);

	camel_folder_summary_clear (summary, NULL);

	if (camel_folder_change_info_changed (changes))
		camel_folder_changed (camel_folder_summary_get_folder (summary), changes);
	camel_folder_change_info_free (changes);
}
