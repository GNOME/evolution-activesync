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

#ifndef _EAS_GET_EMAIL_ATTACHMENT_MSG_H_
#define _EAS_GET_EMAIL_ATTACHMENT_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG             (eas_get_email_attachment_msg_get_type ())
#define EAS_GET_EMAIL_ATTACHMENT_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsg))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))

typedef struct _EasGetEmailAttachmentMsgClass EasGetEmailAttachmentMsgClass;
typedef struct _EasGetEmailAttachmentMsg EasGetEmailAttachmentMsg;
typedef struct _EasGetEmailAttachmentMsgPrivate EasGetEmailAttachmentMsgPrivate;

struct _EasGetEmailAttachmentMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailAttachmentMsg
{
	EasMsgBase parent_instance;
	
	EasGetEmailAttachmentMsgPrivate* priv;
};

GType eas_get_email_attachment_msg_get_type (void) G_GNUC_CONST;

/**
 * Create a new email attachment message.
 *
 * @param[in] fileReference
 *	  Reference used to identify the file on the server. 
 * @param[in] directoryPath
 *	  Full path to the directory the file will be written to on the local file system.
 *
 * @return NULL or a newly created EasGetEmailAttachmentMsg GObject.
 */
EasGetEmailAttachmentMsg* 
eas_get_email_attachment_msg_new (const gchar *fileReference, 
								  const gchar* directoryPath);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasGetEmailAttachmentMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* 
eas_get_email_attachment_msg_build_message (EasGetEmailAttachmentMsg* self);


/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasGetEmailAttachmenentMsg GObject instance 
 * was created.
 *
 * @param[in] self
 *	  The EasGetEmailAttachmentMsg GObject instance.
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
eas_get_email_attachment_msg_parse_response (EasGetEmailAttachmentMsg* self, 
											 xmlDoc* doc, 
											 GError** error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_ATTACHMENT_MSG_H_ */
