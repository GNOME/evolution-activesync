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

#ifndef _EAS_MOVE_EMAIL_REQ_H_
#define _EAS_MOVE_EMAIL_REQ_H_

#include <glib-object.h>

#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_MOVE_EMAIL_REQ             (eas_move_email_req_get_type ())
#define EAS_MOVE_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_MOVE_EMAIL_REQ, EasMoveEmailReq))
#define EAS_MOVE_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_MOVE_EMAIL_REQ, EasMoveEmailReqClass))
#define EAS_IS_MOVE_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_MOVE_EMAIL_REQ))
#define EAS_IS_MOVE_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_MOVE_EMAIL_REQ))
#define EAS_MOVE_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_MOVE_EMAIL_REQ, EasMoveEmailReqClass))

typedef struct _EasMoveEmailReqClass EasMoveEmailReqClass;
typedef struct _EasMoveEmailReq EasMoveEmailReq;
typedef struct _EasMoveEmailReqPrivate EasMoveEmailReqPrivate;

struct _EasMoveEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasMoveEmailReq
{
	EasRequestBase parent_instance;

	EasMoveEmailReqPrivate *priv;
};

GType eas_move_email_req_get_type (void) G_GNUC_CONST;

// C'tor
EasMoveEmailReq *eas_move_email_req_new(const gchar* account_id, EFlag *flag, const GSList* server_ids_list, const gchar* src_folder_id, const gchar* dest_folder_id);

// start async request
gboolean eas_move_email_req_Activate(EasMoveEmailReq *self, GError** error);

// async request completed
void eas_move_email_req_MessageComplete(EasMoveEmailReq *self, xmlDoc* doc, GError* error);

// result returned to client
gboolean eas_move_email_req_ActivateFinish (EasMoveEmailReq* self, GError **error);

G_END_DECLS

#endif /* _EAS_MOVE_EMAIL_REQ_H_ */
