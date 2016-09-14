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

#ifndef CAMEL_GW_SUMMARY_H
#define CAMEL_GW_SUMMARY_H

#include <camel/camel.h>
#include "camel-eas-message-info.h"

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_SUMMARY \
	(camel_eas_summary_get_type ())
#define CAMEL_EAS_SUMMARY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_SUMMARY, CamelEasSummary))
#define CAMEL_EAS_SUMMARY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_SUMMARY, CamelEasSummaryClass))
#define CAMEL_IS_EAS_SUMMARY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_SUMMARY))
#define CAMEL_IS_EAS_SUMMARY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_SUMMARY))
#define CAMEL_EAS_SUMMARY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_SUMMARY, CamelEasSummaryClass))

G_BEGIN_DECLS

typedef struct _CamelEasSummary CamelEasSummary;
typedef struct _CamelEasSummaryClass CamelEasSummaryClass;

struct _CamelEasSummary {
	CamelFolderSummary parent;

	gchar *sync_state;
	gint32 version;
} ;

struct _CamelEasSummaryClass {
	CamelFolderSummaryClass parent_class;
} ;

GType camel_eas_summary_get_type (void);

CamelFolderSummary *
	camel_eas_summary_new		(struct _CamelFolder *folder,
					 const gchar *filename);
gboolean
	camel_eas_update_message_info_flags
					(CamelFolderSummary *summary,
					 CamelMessageInfo *info,
					 guint32 server_flags,
					 const CamelNamedFlags *server_user_flags);
void	camel_eas_summary_add_message	(CamelFolderSummary *summary,
					 const gchar *uid,
					 CamelMimeMessage *message);
void	camel_eas_summary_add_message_info
					(CamelFolderSummary *summary,
					 guint32 server_flags,
					 CamelMessageInfo *info);
void	eas_summary_clear		(CamelFolderSummary *summary,
					 gboolean uncache);

G_END_DECLS

#endif /* CAMEL_GW_SUMMARY_H */
