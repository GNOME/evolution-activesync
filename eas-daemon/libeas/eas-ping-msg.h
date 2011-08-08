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

#ifndef _EAS_SYNC_MSG_H_
#define _EAS_PING_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"
#include "eas-request-base.h"
#include "eas-ping-req.h"

G_BEGIN_DECLS

#define EAS_TYPE_PING_MSG             (eas_ping_msg_get_type ())
#define EAS_PING_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PING_MSG, EasPingMsg))
#define EAS_PING_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PING_MSG, EasPingMsgClass))
#define EAS_IS_PING_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PING_MSG))
#define EAS_IS_PING_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PING_MSG))
#define EAS_PING_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PING_MSG, EasPingMsgClass))

typedef struct _EasPingMsgClass EasPingMsgClass;
typedef struct _EasPingMsg EasPingMsg;
typedef struct _EasPingMsgPrivate EasPingMsgPrivate;

struct _EasPingMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasPingMsg {
	EasMsgBase parent_instance;

	EasPingMsgPrivate *priv;
};

GType eas_ping_msg_get_type (void) G_GNUC_CONST;

EasPingMsg* eas_ping_msg_new ();
xmlDoc* eas_ping_msg_build_message (EasPingMsg* self, const gchar* accountId, const gchar *heartbeat, GSList *folders);
gboolean eas_ping_msg_parse_response (EasPingMsg* self, xmlDoc *doc, GError** error);
GSList* eas_ping_msg_get_changed_folders (EasPingMsg* self);
EasPingReqState eas_ping_msg_get_state (EasPingMsg *self);


G_END_DECLS

#endif /* _EAS_PING_MSG_H_ */
