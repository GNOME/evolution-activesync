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

#ifndef CAMEL_EAS_STORE_SUMMARY_H
#define CAMEL_EAS_STORE_SUMMARY_H

#include <camel/camel.h>
#include <eas-folder.h>

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_STORE_SUMMARY \
	(camel_eas_store_summary_get_type ())
#define CAMEL_EAS_STORE_SUMMARY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_STORE_SUMMARY, CamelEasStoreSummary))
#define CAMEL_EAS_STORE_SUMMARY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_STORE_SUMMARY, CamelEasStoreSummaryClass))
#define CAMEL_IS_EAS_STORE_SUMMARY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_STORE_SUMMARY))
#define CAMEL_IS_EAS_STORE_SUMMARY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_STORE_SUMMARY))
#define CAMEL_EAS_STORE_SUMMARY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_STORE_SUMMARY, CamelEasStoreSummaryClass))

G_BEGIN_DECLS

typedef struct _CamelEasStoreSummary CamelEasStoreSummary;
typedef struct _CamelEasStoreSummaryClass CamelEasStoreSummaryClass;
typedef struct _CamelEasStoreSummaryPrivate CamelEasStoreSummaryPrivate;

struct _CamelEasStoreSummary {
	CamelObject parent;
	CamelEasStoreSummaryPrivate *priv;
};

struct _CamelEasStoreSummaryClass {
	CamelObjectClass parent_class;
};

GType		camel_eas_store_summary_get_type	(void);

CamelEasStoreSummary *
		camel_eas_store_summary_new	(const gchar *path);
gboolean	camel_eas_store_summary_load	(CamelEasStoreSummary *eas_summary,
						 GError **error);
gboolean	camel_eas_store_summary_save	(CamelEasStoreSummary *eas_summary,
						 GError **error);
gboolean	camel_eas_store_summary_clear	(CamelEasStoreSummary *eas_summary);
gboolean	camel_eas_store_summary_remove	(CamelEasStoreSummary *eas_summary);

void		camel_eas_store_summary_set_folder_name
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 const gchar *display_name);
void		camel_eas_store_summary_set_parent_folder_id
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 const gchar *parent_id);
void		camel_eas_store_summary_set_folder_type
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 guint64 folder_type);

gchar *	camel_eas_store_summary_get_folder_name
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 GError **error);
gchar *camel_eas_store_summary_get_folder_full_name
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 GError **error);
gchar *	camel_eas_store_summary_get_parent_folder_id
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 GError **error);
guint64		camel_eas_store_summary_get_folder_type
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 GError **error);

GSList *	camel_eas_store_summary_get_folders
						(CamelEasStoreSummary *eas_summary,
						 const gchar *prefix);


void		camel_eas_store_summary_store_string_val
						(CamelEasStoreSummary *eas_summary,
						 const gchar *key,
						 const gchar *value);

gchar *	camel_eas_store_summary_get_string_val
						(CamelEasStoreSummary *eas_summary,
						 const gchar *key,
						 GError **error);

gboolean	camel_eas_store_summary_remove_folder
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_id,
						 GError **error);

void		camel_eas_store_summary_new_folder
						(CamelEasStoreSummary *eas_summary,
						 const EasFolder *folder);

gchar *		camel_eas_store_summary_get_folder_id_from_name
						(CamelEasStoreSummary *eas_summary,
						 const gchar *folder_name);

gboolean	camel_eas_store_summary_has_folder
						(CamelEasStoreSummary *eas_summary,
						 const gchar *id);

G_END_DECLS

#endif /* CAMEL_EAS_STORE_SUMMARY_H */
