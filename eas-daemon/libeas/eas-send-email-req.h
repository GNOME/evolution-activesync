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

#ifndef _EAS_SEND_EMAIL_REQ_H_
#define _EAS_SEND_EMAIL_REQ_H_

#include <glib-object.h>

#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SEND_EMAIL_REQ             (eas_send_email_req_get_type ())
#define EAS_SEND_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReq))
#define EAS_SEND_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqClass))
#define EAS_IS_SEND_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SEND_EMAIL_REQ))
#define EAS_IS_SEND_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SEND_EMAIL_REQ))
#define EAS_SEND_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqClass))

typedef struct _EasSendEmailReqClass EasSendEmailReqClass;
typedef struct _EasSendEmailReq EasSendEmailReq;
typedef struct _EasSendEmailReqPrivate EasSendEmailReqPrivate;

struct _EasSendEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSendEmailReq
{
	EasRequestBase parent_instance;

	EasSendEmailReqPrivate *priv;
};

GType eas_send_email_req_get_type (void) G_GNUC_CONST;

// C'tor
EasSendEmailReq *eas_send_email_req_new();

// TODO - move params out of here and into c'tor:
// start async request
void eas_send_email_req_Activate(EasSendEmailReq *self, guint64 accountID, EFlag *flag, const gchar* clientid, const gchar* mime_file, EasItemType type, GError** error);

// async request completed
void eas_send_email_req_MessageComplete(EasSendEmailReq *self, xmlDoc* doc, GError** error);

// no Finalise method since there are no results returned to client

G_END_DECLS

#endif /* _EAS_SEND_EMAIL_REQ_H_ */
