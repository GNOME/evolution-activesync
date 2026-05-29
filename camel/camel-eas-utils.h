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

#ifndef CAMEL_EAS_UTILS_H
#define CAMEL_EAS_UTILS_H

#include <camel/camel.h>
#include "camel-eas-store.h"
#include "camel-eas-folder.h"

/*Headers for send options*/
#define X_SEND_OPTIONS        "X-eas-send-options"
/*General Options*/
#define X_SEND_OPT_PRIORITY   "X-eas-send-opt-priority"
#define X_SEND_OPT_SECURITY   "X-eas-send-opt-security"
#define X_REPLY_CONVENIENT    "X-reply-convenient"
#define X_REPLY_WITHIN        "X-reply-within"
#define X_EXPIRE_AFTER        "X-expire-after"
#define X_DELAY_UNTIL         "X-delay-until"

/*Status Tracking Options*/
#define X_TRACK_WHEN            "X-track-when"
#define X_AUTODELETE            "X-auto-delete"
#define X_RETURN_NOTIFY_OPEN    "X-return-notify-open"
#define X_RETURN_NOTIFY_DELETE  "X-return-notify-delete"

/* Folder types for source */
#define RECEIVED  "Mailbox"
#define SENT	  "Sent Items"
#define DRAFT	  ""
#define PERSONAL  "Cabinet"

G_BEGIN_DECLS

/*for syncing flags back to server*/
typedef struct {
	guint32 changed;
	guint32 bits;
} flags_diff_t;

void	eas_utils_sync_folders	(CamelEasStore *eas_store,
				 GSList *folder_list);

CamelFolderInfo *
	camel_eas_utils_build_folder_info	(CamelEasStore *store,
						 const gchar *fname);

void	camel_eas_utils_clear_folder (CamelEasFolder *eas_folder);

int	camel_eas_utils_sync_deleted_items	(CamelEasFolder *eas_folder,
						 GSList *items_deleted);
int	camel_eas_utils_sync_created_items	(CamelEasFolder *eas_folder,
						 GSList *items_created);
int	camel_eas_utils_sync_updated_items	(CamelEasFolder *eas_folder,
						 GSList *items_updated);

G_END_DECLS

#endif
