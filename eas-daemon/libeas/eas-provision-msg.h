/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */
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

#ifndef _EAS_PROVISION_MSG_H_
#define _EAS_PROVISION_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"
#include "eas-provision-list.h"

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_MSG             (eas_provision_msg_get_type ())
#define EAS_PROVISION_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsg))
#define EAS_PROVISION_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))
#define EAS_IS_PROVISION_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_MSG))
#define EAS_IS_PROVISION_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_MSG))
#define EAS_PROVISION_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))

typedef struct _EasProvisionMsgClass EasProvisionMsgClass;
typedef struct _EasProvisionMsg EasProvisionMsg;
typedef struct _EasProvisionMsgPrivate EasProvisionMsgPrivate;

struct _EasProvisionMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasProvisionMsg {
	EasMsgBase parent_instance;

	EasProvisionMsgPrivate *priv;
};

GType eas_provision_msg_get_type (void) G_GNUC_CONST;


/**
 * Create a new provisioning message.
 *
 * @return NULL or a newly created EasProvisionMsg GObject.
 */
EasProvisionMsg* eas_provision_msg_new (void);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* eas_provision_msg_build_message (EasProvisionMsg* self);

/**
 * Parses the response from the server, storing the email attachment according
 * to the parameters set when the EasProvisionMsg GObject instance was
 * created.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
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
eas_provision_msg_parse_response (EasProvisionMsg* self,
				  xmlDoc* doc,
				  GError** error);

/**
 * Getter for policy key.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or the policy_key. [no transfer]
 */
gchar* eas_provision_msg_get_policy_key (EasProvisionMsg* self);

/**
 * Getter for policy status.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or the policy_status. [no transfer]
 */
gchar* eas_provision_msg_get_policy_status (EasProvisionMsg* self);

/**
 * Getter for provision list
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or the policy list. [no transfer]
 */
EasProvisionList* eas_provision_msg_get_provision_list (EasProvisionMsg* self);

/**
 * Setter for policy key.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 * @param[in] policyKey
 *	  New policy key. [no transfer]
 */
void eas_provision_msg_set_policy_key (EasProvisionMsg* self, const gchar* policyKey);

/**
 * Setter for policy status.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 * @param[in] policyStatus
 *	  New policy status. [no transfer]
 */
void eas_provision_msg_set_policy_status (EasProvisionMsg* self, const gchar* policyStatus);

G_END_DECLS

#endif /* _EAS_PROVISION_MSG_H_ */
