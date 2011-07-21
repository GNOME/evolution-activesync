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

#ifndef _EAS_GET_ITEM_ESTIMATE_MSG_H_
#define _EAS_GET_ITEM_ESTIMATE_MSG_H_

#include <glib-object.h>
#include "eas-connection.h"
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_ITEM_ESTIMATE_MSG             (eas_get_item_estimate_msg_get_type ())
#define EAS_GET_ITEM_ESTIMATE_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_MSG, EasGetItemEstimateMsg))
#define EAS_GET_ITEM_ESTIMATE_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_ITEM_ESTIMATE_MSG, EasGetItemEstimateMsgClass))
#define EAS_IS_GET_ITEM_ESTIMATE_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_MSG))
#define EAS_IS_GET_ITEM_ESTIMATE_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_ITEM_ESTIMATE_MSG))
#define EAS_GET_ITEM_ESTIMATE_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_ITEM_ESTIMATE_MSG, EasGetItemEstimateMsgClass))

typedef struct _EasGetItemEstimateMsgClass EasGetItemEstimateMsgClass;
typedef struct _EasGetItemEstimateMsg EasGetItemEstimateMsg;
typedef struct _EasGetItemEstimateMsgPrivate EasGetItemEstimateMsgPrivate;

struct _EasGetItemEstimateMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetItemEstimateMsg
{
	EasMsgBase parent_instance;

	EasGetItemEstimateMsgPrivate *priv;
};

GType eas_get_item_estimate_msg_get_type (void) G_GNUC_CONST;

// c'tor
EasGetItemEstimateMsg* eas_get_item_estimate_msg_new (EasConnection *conn, const gchar* sync_key, const gchar* src_folder_id);

// build xml for GetItemEstimate request
xmlDoc* eas_get_item_estimate_msg_build_message (EasGetItemEstimateMsg* self);

// parse response to GetItemEstimate
gboolean eas_get_item_estimate_msg_parse_response (EasGetItemEstimateMsg* self, xmlDoc *doc, GError** error);

// get the updated ids
guint eas_get_item_estimate_get_estimate (EasGetItemEstimateMsg* self);

G_END_DECLS

#endif /* _EAS_GET_ITEM_ESTIMATE_MSG_H_ */
