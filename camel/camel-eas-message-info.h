/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2016 Red Hat, Inc. (www.redhat.com)
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAMEL_EAS_MESSAGE_INFO_H
#define CAMEL_EAS_MESSAGE_INFO_H

#include <glib-object.h>

#include <camel/camel.h>

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_MESSAGE_INFO \
	(camel_eas_message_info_get_type ())
#define CAMEL_EAS_MESSAGE_INFO(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_MESSAGE_INFO, CamelEasMessageInfo))
#define CAMEL_EAS_MESSAGE_INFO_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_MESSAGE_INFO, CamelEasMessageInfoClass))
#define CAMEL_IS_EAS_MESSAGE_INFO(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_MESSAGE_INFO))
#define CAMEL_IS_EAS_MESSAGE_INFO_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_MESSAGE_INFO))
#define CAMEL_EAS_MESSAGE_INFO_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_MESSAGE_INFO, CamelEasMessageInfoClass))

G_BEGIN_DECLS

/* extra summary flags*/
enum {
	CAMEL_EAS_MESSAGE_MSGFLAG_RN_PENDING = CAMEL_MESSAGE_FOLDER_FLAGGED << 1
};

typedef struct _CamelEasMessageInfo CamelEasMessageInfo;
typedef struct _CamelEasMessageInfoClass CamelEasMessageInfoClass;
typedef struct _CamelEasMessageInfoPrivate CamelEasMessageInfoPrivate;

struct _CamelEasMessageInfo {
	CamelMessageInfoBase parent;
	CamelEasMessageInfoPrivate *priv;
};

struct _CamelEasMessageInfoClass {
	CamelMessageInfoBaseClass parent_class;
};

GType		camel_eas_message_info_get_type	(void);

guint32		camel_eas_message_info_get_server_flags	(const CamelEasMessageInfo *emi);
gboolean	camel_eas_message_info_set_server_flags	(CamelEasMessageInfo *emi,
							 guint32 server_flags);
gint32		camel_eas_message_info_get_item_type	(const CamelEasMessageInfo *emi);
gboolean	camel_eas_message_info_set_item_type	(CamelEasMessageInfo *emi,
							 gint32 item_type);
const gchar *	camel_eas_message_info_get_change_key	(const CamelEasMessageInfo *emi);
gchar *		camel_eas_message_info_dup_change_key	(const CamelEasMessageInfo *emi);
gboolean	camel_eas_message_info_set_change_key	(CamelEasMessageInfo *emi,
							 const gchar *change_key);
gboolean	camel_eas_message_info_take_change_key	(CamelEasMessageInfo *emi,
							 gchar *change_key);

G_END_DECLS

#endif /* CAMEL_EAS_MESSAGE_INFO_H */
