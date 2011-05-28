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

#ifndef _EAS_UPDATE_EMAIL_REQ_H_
#define _EAS_UPDATE_EMAIL_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_UPDATE_EMAIL_REQ             (eas_update_email_req_get_type ())
#define EAS_UPDATE_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReq))
#define EAS_UPDATE_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqClass))
#define EAS_IS_UPDATE_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_UPDATE_EMAIL_REQ))
#define EAS_IS_UPDATE_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_UPDATE_EMAIL_REQ))
#define EAS_UPDATE_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqClass))

typedef struct _EasUpdateEmailReqClass EasUpdateEmailReqClass;
typedef struct _EasUpdateEmailReq EasUpdateEmailReq;
typedef struct _EasUpdateEmailReqPrivate EasUpdateEmailReqPrivate;

struct _EasUpdateEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasUpdateEmailReq
{
	EasRequestBase parent_instance;

	EasUpdateEmailReqPrivate * priv;
};

GType eas_update_email_req_get_type (void) G_GNUC_CONST;

// C'tor
EasUpdateEmailReq *eas_update_email_req_new(guint64 account_id, const gchar *sync_key, const gchar *folder_id, const gchar *serialised_email, EFlag *flag);

// start async request
void eas_update_email_req_Activate(EasUpdateEmailReq *self);

// async request completed
void eas_update_email_req_MessageComplete(EasUpdateEmailReq *self, xmlDoc* doc);

// results returned to client
void eas_update_email_req_ActivateFinish (EasUpdateEmailReq* self, gchar** ret_sync_key);

G_END_DECLS

#endif /* _EAS_UPDATE_EMAIL_REQ_H_ */
