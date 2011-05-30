/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 */

#ifndef _EAS_GET_EMAIL_ATTACHMENT_REQ_H_
#define _EAS_GET_EMAIL_ATTACHMENT_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"
G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ             (eas_get_email_attachment_req_get_type ())
#define EAS_GET_EMAIL_ATTACHMENT_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReq))
#define EAS_GET_EMAIL_ATTACHMENT_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqClass))
#define EAS_IS_GET_EMAIL_ATTACHMENT_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ))
#define EAS_IS_GET_EMAIL_ATTACHMENT_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ))
#define EAS_GET_EMAIL_ATTACHMENT_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqClass))

typedef struct _EasGetEmailAttachmentReqClass EasGetEmailAttachmentReqClass;
typedef struct _EasGetEmailAttachmentReq EasGetEmailAttachmentReq;
typedef struct _EasGetEmailAttachmentReqPrivate EasGetEmailAttachmentReqPrivate;

struct _EasGetEmailAttachmentReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasGetEmailAttachmentReq
{
	EasRequestBase parent_instance;
	
	EasGetEmailAttachmentReqPrivate* priv;
};

GType eas_get_email_attachment_req_get_type (void) G_GNUC_CONST;

EasGetEmailAttachmentReq* 
eas_get_email_attachment_req_new (const guint64 account_uid, 
                            const gchar *file_reference,
                            const gchar *mime_directory,
                            EFlag *flag);
                   
void eas_get_email_attachment_req_Activate (EasGetEmailAttachmentReq* self);
void eas_get_email_attachment_req_MessageComplete (EasGetEmailAttachmentReq* self, xmlDoc *doc);
void eas_get_email_attachment_req_ActivateFinish (EasGetEmailAttachmentReq* self, GError **error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_ATTACHMENT_REQ_H_ */
