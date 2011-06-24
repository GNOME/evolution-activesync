/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_GET_EMAIL_BODY_REQ_H_
#define _EAS_GET_EMAIL_BODY_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_BODY_REQ             (eas_get_email_body_req_get_type ())
#define EAS_GET_EMAIL_BODY_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReq))
#define EAS_GET_EMAIL_BODY_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReqClass))
#define EAS_IS_GET_EMAIL_BODY_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_BODY_REQ))
#define EAS_IS_GET_EMAIL_BODY_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_BODY_REQ))
#define EAS_GET_EMAIL_BODY_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_BODY_REQ, EasGetEmailBodyReqClass))

typedef struct _EasGetEmailBodyReqClass EasGetEmailBodyReqClass;
typedef struct _EasGetEmailBodyReq EasGetEmailBodyReq;
typedef struct _EasGetEmailBodyReqPrivate EasGetEmailBodyReqPrivate;

struct _EasGetEmailBodyReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasGetEmailBodyReq
{
	EasRequestBase parent_instance;

	EasGetEmailBodyReqPrivate* priv;
};

GType eas_get_email_body_req_get_type (void) G_GNUC_CONST;

EasGetEmailBodyReq* 
eas_get_email_body_req_new (const gchar* account_uid, 
                            const gchar *collection_id, 
                            const gchar *server_id, 
                            const gchar *mime_directory,
                            DBusGMethodInvocation *context);

gboolean eas_get_email_body_req_Activate (EasGetEmailBodyReq* self, GError** error);
gboolean eas_get_email_body_req_MessageComplete (EasGetEmailBodyReq* self, xmlDoc *doc, GError* error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_BODY_REQ_H_ */
