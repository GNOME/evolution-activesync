/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_DELETE_EMAIL_REQ_H_
#define _EAS_DELETE_EMAIL_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_DELETE_EMAIL_REQ             (eas_delete_email_req_get_type ())
#define EAS_DELETE_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReq))
#define EAS_DELETE_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqClass))
#define EAS_IS_DELETE_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_DELETE_EMAIL_REQ))
#define EAS_IS_DELETE_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_DELETE_EMAIL_REQ))
#define EAS_DELETE_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqClass))

typedef struct _EasDeleteEmailReqClass EasDeleteEmailReqClass;
typedef struct _EasDeleteEmailReq EasDeleteEmailReq;
typedef struct _EasDeleteEmailReqPrivate EasDeleteEmailReqPrivate;

struct _EasDeleteEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasDeleteEmailReq
{
	EasRequestBase parent_instance;

	EasDeleteEmailReqPrivate* priv;	
};

GType eas_delete_email_req_get_type (void) G_GNUC_CONST;
EasDeleteEmailReq *eas_delete_email_req_new (guint64 accountId, const gchar *syncKey, const gchar *folderId, const GSList *server_ids_array, EFlag *flag);
void eas_delete_email_req_Activate (EasDeleteEmailReq *self, GError** error);
void eas_delete_email_req_ActivateFinish (EasDeleteEmailReq* self, gchar** ret_sync_key, GError** error);
void eas_delete_email_req_MessageComplete (EasDeleteEmailReq *self, xmlDoc* doc, GError** error);

G_END_DECLS

#endif /* _EAS_DELETE_EMAIL_REQ_H_ */
