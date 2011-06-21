/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EAS_MOVE_EMAIL_MSG_H_
#define _EAS_MOVE_EMAIL_MSG_H_

#include <glib-object.h>

#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_MOVE_EMAIL_MSG             (eas_move_email_msg_get_type ())
#define EAS_MOVE_EMAIL_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_MOVE_EMAIL_MSG, EasMoveEmailMsg))
#define EAS_MOVE_EMAIL_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_MOVE_EMAIL_MSG, EasMoveEmailMsgClass))
#define EAS_IS_MOVE_EMAIL_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_MOVE_EMAIL_MSG))
#define EAS_IS_MOVE_EMAIL_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_MOVE_EMAIL_MSG))
#define EAS_MOVE_EMAIL_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_MOVE_EMAIL_MSG, EasMoveEmailMsgClass))

typedef struct _EasMoveEmailMsgClass EasMoveEmailMsgClass;
typedef struct _EasMoveEmailMsg EasMoveEmailMsg;
typedef struct _EasMoveEmailMsgPrivate EasMoveEmailMsgPrivate;

struct _EasMoveEmailMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasMoveEmailMsg
{
	EasMsgBase parent_instance;

	EasMoveEmailMsgPrivate *priv;
};

GType eas_move_email_msg_get_type (void) G_GNUC_CONST;

// c'tor
EasMoveEmailMsg* eas_move_email_msg_new (const char* account_id, const GSList* server_ids_list, const gchar* src_folder_id, const gchar* dest_folder_id);

// build xml for MoveItems request
xmlDoc* eas_move_email_msg_build_message (EasMoveEmailMsg* self);

// parse response to MoveItems
gboolean eas_move_email_msg_parse_response (EasMoveEmailMsg* self, xmlDoc *doc, GError** error);

G_END_DECLS

#endif /* _EAS_MOVE_EMAIL_MSG_H_ */
