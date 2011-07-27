/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef _EAS_GET_EMAIL_BODY_MSG_H_
#define _EAS_GET_EMAIL_BODY_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_BODY_MSG             (eas_get_email_body_msg_get_type ())
#define EAS_GET_EMAIL_BODY_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsg))
#define EAS_GET_EMAIL_BODY_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))
#define EAS_IS_GET_EMAIL_BODY_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_IS_GET_EMAIL_BODY_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_GET_EMAIL_BODY_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))

typedef struct _EasGetEmailBodyMsgClass EasGetEmailBodyMsgClass;
typedef struct _EasGetEmailBodyMsg EasGetEmailBodyMsg;
typedef struct _EasGetEmailBodyMsgPrivate EasGetEmailBodyMsgPrivate;

struct _EasGetEmailBodyMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailBodyMsg
{
	EasMsgBase parent_instance;

	EasGetEmailBodyMsgPrivate* priv;
};

GType eas_get_email_body_msg_get_type (void) G_GNUC_CONST;


/**
 * Create a new email body message.
 *
 * @param[in] accountUid
 *	  Unique identifier for a user account.
 * @param[in] collectionId
 *	  The identifer for the target server folder.
 * @param[in] directoryPath
 *	  Full path to the directory the file will be written to on the local file system.
 *
 * @return NULL or a newly created EasGetEmailAttachmentMsg GObject.
 */
EasGetEmailBodyMsg* 
eas_get_email_body_msg_new (const gchar* serverUid, 
							const gchar* collectionId, 
							const gchar* directoryPath);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasGetEmailBodyMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* 
eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self);

/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasGetEmailBodyMsg GObject instance was 
 * created.
 *
 * @param[in] self
 *	  The EasGetEmailBodyMsg GObject instance.
 * @param[in] doc
 *	  libxml DOM tree structure containing the XML to be parsed. [no transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean 
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, 
									   xmlDoc* doc, 
									   GError** error);

gchar* 
eas_get_email_body_msg_get_item (EasGetEmailBodyMsg* self);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_BODY_MSG_H_ */
