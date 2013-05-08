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

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>
#include "camel-eas-store-summary.h"

#define S_LOCK(x) (g_rec_mutex_lock(&(x)->priv->s_lock))
#define S_UNLOCK(x) (g_rec_mutex_unlock(&(x)->priv->s_lock))

#define STORE_GROUP_NAME "##storepriv"

struct _CamelEasStoreSummaryPrivate {
	GKeyFile *key_file;
	gboolean dirty;
	gchar *path;
	/* Note: We use the *same* strings in both of these hash tables, and
	   only id_fname_hash has g_free() hooked up as the destructor func.
	   So entries must always be removed from fname_id_hash *first*. */
	GHashTable *id_fname_hash;
	GHashTable *fname_id_hash;
	GRecMutex s_lock;
};

G_DEFINE_TYPE (CamelEasStoreSummary, camel_eas_store_summary, CAMEL_TYPE_OBJECT)

static void
eas_store_summary_finalize (GObject *object)
{
	CamelEasStoreSummary *eas_summary = CAMEL_EAS_STORE_SUMMARY (object);
	CamelEasStoreSummaryPrivate *priv = eas_summary->priv;

	g_key_file_free (priv->key_file);
	g_free (priv->path);
	g_hash_table_destroy (priv->fname_id_hash);
	g_hash_table_destroy (priv->id_fname_hash);

	g_free (priv);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (camel_eas_store_summary_parent_class)->finalize (object);
}

static void
camel_eas_store_summary_class_init (CamelEasStoreSummaryClass *class)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (class);
	object_class->finalize = eas_store_summary_finalize;
}

static void
camel_eas_store_summary_init (CamelEasStoreSummary *eas_summary)
{
	CamelEasStoreSummaryPrivate *priv;

	priv = g_new0 (CamelEasStoreSummaryPrivate, 1);
	eas_summary->priv = priv;

	priv->key_file = g_key_file_new ();
	priv->dirty = FALSE;
	priv->fname_id_hash = g_hash_table_new (g_str_hash, g_str_equal);
	priv->id_fname_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
						     (GDestroyNotify) g_free,
						     (GDestroyNotify) g_free);
	g_rec_mutex_init (&priv->s_lock);
}

static gchar *build_full_name (CamelEasStoreSummary *eas_summary, const gchar *fid)
{
	gchar *pfid, *dname, *ret;
	gchar *pname = NULL;

	dname = camel_eas_store_summary_get_folder_name (eas_summary, fid, NULL);
	if (!dname)
		return NULL;

	pfid = camel_eas_store_summary_get_parent_folder_id (eas_summary, fid, NULL);
	if (pfid) {
		pname = build_full_name (eas_summary, pfid);
		g_free (pfid);
	}

	if (pname) {
		ret = g_strdup_printf ("%s/%s", pname, dname);
		g_free (pname);
		g_free (dname);
	} else
		ret = dname;

	return ret;
}

static void
load_id_fname_hash (CamelEasStoreSummary *eas_summary)
{
	GSList *folders, *l;

	folders = camel_eas_store_summary_get_folders (eas_summary, NULL);

	for (l = folders; l != NULL; l = g_slist_next (l)) {
		gchar *id = l->data;
		gchar *fname;

		fname = build_full_name (eas_summary, id);

		g_hash_table_insert (eas_summary->priv->fname_id_hash, fname, id);
		g_hash_table_insert (eas_summary->priv->id_fname_hash, id, fname);
	}

	g_slist_free (folders);
}

CamelEasStoreSummary *
camel_eas_store_summary_new (const gchar *path)
{
	CamelEasStoreSummary *eas_summary;

	eas_summary = g_object_new (CAMEL_TYPE_EAS_STORE_SUMMARY, NULL);

	eas_summary->priv->path = g_strdup (path);

	return eas_summary;
}

gboolean
camel_eas_store_summary_load (CamelEasStoreSummary *eas_summary,
			      GError **error)
{
	CamelEasStoreSummaryPrivate *priv = eas_summary->priv;
	gboolean ret;

	S_LOCK(eas_summary);

	ret = g_key_file_load_from_file	(priv->key_file,
					 priv->path,
					 0, error);

	load_id_fname_hash (eas_summary);
	S_UNLOCK(eas_summary);

	return ret;
}

gboolean
camel_eas_store_summary_save (CamelEasStoreSummary *eas_summary,
			      GError **error)
{
	CamelEasStoreSummaryPrivate *priv = eas_summary->priv;
	gboolean ret = TRUE;
	GFile *file;
	gchar *contents = NULL;

	S_LOCK(eas_summary);

	if (!priv->dirty)
		goto exit;

	contents = g_key_file_to_data	(priv->key_file, NULL,
		       			 NULL);
	file = g_file_new_for_path	(priv->path);
	ret = g_file_replace_contents	(file, contents, strlen (contents),
					 NULL, FALSE, G_FILE_CREATE_PRIVATE,
					 NULL, NULL, error);
	g_object_unref (file);
	priv->dirty = FALSE;

exit:
	S_UNLOCK(eas_summary);

	g_free (contents);
	return ret;
}

gboolean
camel_eas_store_summary_clear (CamelEasStoreSummary *eas_summary)
{

	S_LOCK(eas_summary);

	g_key_file_free (eas_summary->priv->key_file);
	eas_summary->priv->key_file = g_key_file_new ();
	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);

	return TRUE;
}

gboolean
camel_eas_store_summary_remove (CamelEasStoreSummary *eas_summary)
{
	gint ret;

	S_LOCK(eas_summary);

	if (eas_summary->priv->key_file)
		camel_eas_store_summary_clear (eas_summary);

	ret = g_unlink (eas_summary->priv->path);

	S_UNLOCK(eas_summary);

	return (ret == 0);
}

struct subfolder_match {
	GSList *ids;
	gchar *match;
	gsize matchlen;
};

static void
match_subfolder (gpointer key, gpointer value, gpointer user_data)
{
	struct subfolder_match *sm = user_data;

	if (!strncmp (key, sm->match, sm->matchlen))
		sm->ids = g_slist_prepend (sm->ids, g_strdup (value));
}

/* Must be called with the summary lock held, and gets to keep
   both its string arguments */
static void eas_ss_hash_replace (CamelEasStoreSummary *eas_summary,
				 gchar *folder_id, gchar *full_name,
				 gboolean recurse)
{
	const gchar *ofname;
	struct subfolder_match sm = { NULL, NULL };

	if (!full_name)
		full_name = build_full_name (eas_summary, folder_id);

	ofname = g_hash_table_lookup (eas_summary->priv->id_fname_hash,
				      folder_id);
	/* Remove the old fullname->id hash entry *iff* it's pointing
	   to this folder id. */
	if (ofname) {
		char *ofid = g_hash_table_lookup (eas_summary->priv->fname_id_hash,
						  ofname);
		if (!strcmp (folder_id, ofid)) {
			g_hash_table_remove (eas_summary->priv->fname_id_hash,
					     ofname);
			if (recurse)
				sm.match = g_strdup_printf ("%s/", ofname);
		}
	}

	g_hash_table_insert (eas_summary->priv->fname_id_hash, full_name, folder_id);

	/* Replace, not insert. The difference is that it frees the *old* folder_id
	   key, not the new one which we just inserted into fname_id_hash too. */
	g_hash_table_replace (eas_summary->priv->id_fname_hash, folder_id, full_name);

	if (sm.match) {
		GSList *l;

		sm.matchlen = strlen (sm.match);

		g_hash_table_foreach (eas_summary->priv->fname_id_hash,
				      match_subfolder, &sm);

		for (l = sm.ids; l; l = g_slist_next (l))
			eas_ss_hash_replace (eas_summary, l->data, NULL, FALSE);

		g_slist_free (sm.ids);
		g_free (sm.match);
	}
}

void
camel_eas_store_summary_set_folder_name (CamelEasStoreSummary *eas_summary,
					 const gchar *folder_id,
					 const gchar *display_name)
{
	S_LOCK(eas_summary);

	g_key_file_set_string	(eas_summary->priv->key_file, folder_id,
				 "DisplayName", display_name);

	eas_ss_hash_replace (eas_summary, g_strdup (folder_id), NULL, TRUE);
	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);
}


void
camel_eas_store_summary_new_folder (CamelEasStoreSummary *eas_summary,
				    const EasFolder *folder)
{
	S_LOCK(eas_summary);

	g_key_file_set_string (eas_summary->priv->key_file, folder->folder_id,
			       "ParentFolderId", folder->parent_id);
	g_key_file_set_string (eas_summary->priv->key_file, folder->folder_id,
			       "DisplayName", folder->display_name);
	g_key_file_set_uint64 (eas_summary->priv->key_file, folder->folder_id,
			       "FolderType", folder->type);

	eas_ss_hash_replace (eas_summary, g_strdup (folder->folder_id), NULL, FALSE);

	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);
}


void
camel_eas_store_summary_set_parent_folder_id (CamelEasStoreSummary *eas_summary,
					      const gchar *folder_id,
					      const gchar *parent_id)
{
	S_LOCK(eas_summary);

	if (parent_id)
		g_key_file_set_string (eas_summary->priv->key_file, folder_id,
				       "ParentFolderId", parent_id);
	else
		g_key_file_remove_key (eas_summary->priv->key_file, folder_id,
				       "ParentFolderId", NULL);

	eas_ss_hash_replace (eas_summary, g_strdup (folder_id), NULL, TRUE);

	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);
}

void
camel_eas_store_summary_set_folder_type (CamelEasStoreSummary *eas_summary,
					 const gchar *folder_id,
					 guint64 eas_folder_type)
{
	S_LOCK(eas_summary);

	g_key_file_set_uint64	(eas_summary->priv->key_file, folder_id,
				 "FolderType", eas_folder_type);
	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);
}

void
camel_eas_store_summary_store_string_val (CamelEasStoreSummary *eas_summary,
					  const gchar *key,
					  const gchar *value)
{
	S_LOCK(eas_summary);

	g_key_file_set_string	(eas_summary->priv->key_file, STORE_GROUP_NAME,
				 key, value);
	eas_summary->priv->dirty = TRUE;

	S_UNLOCK(eas_summary);
}

gchar *
camel_eas_store_summary_get_folder_name (CamelEasStoreSummary *eas_summary,
					 const gchar *folder_id,
					 GError **error)
{
	gchar *ret;

	S_LOCK(eas_summary);

	ret = g_key_file_get_string	(eas_summary->priv->key_file, folder_id,
					 "DisplayName", error);

	S_UNLOCK(eas_summary);

	return ret;
}

gchar *
camel_eas_store_summary_get_folder_full_name (CamelEasStoreSummary *eas_summary,
					      const gchar *folder_id,
					      GError **error)
{
	gchar *ret;

	S_LOCK(eas_summary);

	ret = g_hash_table_lookup (eas_summary->priv->id_fname_hash, folder_id);

	if (ret)
		ret = g_strdup (ret);

	S_UNLOCK(eas_summary);

	return ret;
}

gchar *
camel_eas_store_summary_get_parent_folder_id (CamelEasStoreSummary *eas_summary,
					      const gchar *folder_id,
					      GError **error)
{
	gchar *ret;

	S_LOCK(eas_summary);

	ret = g_key_file_get_string	(eas_summary->priv->key_file, folder_id,
					 "ParentFolderId", error);

	S_UNLOCK(eas_summary);

	return ret;
}

guint64
camel_eas_store_summary_get_folder_type (CamelEasStoreSummary *eas_summary,
					 const gchar *folder_id,
					 GError **error)
{
	guint64 ret;

	S_LOCK(eas_summary);

	ret = g_key_file_get_uint64	(eas_summary->priv->key_file, folder_id,
					 "FolderType", error);

	S_UNLOCK(eas_summary);

	return ret;
}

gchar *
camel_eas_store_summary_get_string_val	(CamelEasStoreSummary *eas_summary,
					 const gchar *key,
					 GError **error)
{
	gchar *ret;

	S_LOCK(eas_summary);

	ret = g_key_file_get_string	(eas_summary->priv->key_file, STORE_GROUP_NAME,
					 key, error);

	S_UNLOCK(eas_summary);

	return ret;
}

GSList *
camel_eas_store_summary_get_folders (CamelEasStoreSummary *eas_summary,
				     const gchar *prefix)
{
	GSList *folders = NULL;
	gchar **groups = NULL;
	gsize length;
	int prefixlen = 0;
	gint i;

	if (prefix)
		prefixlen = strlen(prefix);

	S_LOCK(eas_summary);

	groups = g_key_file_get_groups (eas_summary->priv->key_file, &length);

	S_UNLOCK(eas_summary);

	for (i = 0; i < length; i++) {
		if (!g_ascii_strcasecmp (groups [i], STORE_GROUP_NAME))
			continue;
		if (prefixlen) {
			const gchar *fname = g_hash_table_lookup (eas_summary->priv->id_fname_hash, groups[i]);

			if (!fname || strncmp(fname, prefix, prefixlen) ||
			    (fname[prefixlen] && fname[prefixlen] != '/'))
				continue;
		}
		folders = g_slist_append (folders, g_strdup (groups [i]));
	}

	g_strfreev (groups);
	return folders;
}

gboolean
camel_eas_store_summary_remove_folder (CamelEasStoreSummary *eas_summary,
				       const gchar *folder_id,
				       GError **error)
{
	gboolean ret = FALSE;
	gchar *full_name;

	S_LOCK(eas_summary);

	full_name = g_hash_table_lookup (eas_summary->priv->id_fname_hash, folder_id);
	if (!full_name)
		goto unlock;

	ret = g_key_file_remove_group (eas_summary->priv->key_file, folder_id,
				       error);

	g_hash_table_remove (eas_summary->priv->fname_id_hash, full_name);
	g_hash_table_remove (eas_summary->priv->id_fname_hash, folder_id);

	eas_summary->priv->dirty = TRUE;

 unlock:
	S_UNLOCK(eas_summary);

	return ret;
}

gchar *
camel_eas_store_summary_get_folder_id_from_name (CamelEasStoreSummary *eas_summary,
						 const gchar *folder_name)
{
	gchar *folder_id;

	S_LOCK(eas_summary);

	folder_id = g_hash_table_lookup (eas_summary->priv->fname_id_hash, folder_name);
	if (folder_id)
		folder_id = g_strdup (folder_id);

	S_UNLOCK(eas_summary);

	return folder_id;
}

gboolean
camel_eas_store_summary_has_folder (CamelEasStoreSummary *eas_summary, const gchar *folder_id)
{
	gboolean ret;

	S_LOCK(eas_summary);

	ret = g_key_file_has_group (eas_summary->priv->key_file, folder_id);

	S_UNLOCK(eas_summary);

	return ret;
}
