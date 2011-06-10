/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_PROVISION_REQ_H_
#define _EAS_PROVISION_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_REQ             (eas_provision_req_get_type ())
#define EAS_PROVISION_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_REQ, EasProvisionReq))
#define EAS_PROVISION_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_REQ, EasProvisionReqClass))
#define EAS_IS_PROVISION_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_REQ))
#define EAS_IS_PROVISION_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_REQ))
#define EAS_PROVISION_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_REQ, EasProvisionReqClass))

typedef struct _EasProvisionReqClass EasProvisionReqClass;
typedef struct _EasProvisionReq EasProvisionReq;
typedef struct _EasProvisionReqPrivate EasProvisionReqPrivate;

struct _EasProvisionReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasProvisionReq
{
	EasRequestBase parent_instance;

	EasProvisionReqPrivate* priv;
};

GType eas_provision_req_get_type (void) G_GNUC_CONST;
EasProvisionReq* eas_provision_req_new (gchar* policy_status, gchar* policy_key);
gboolean eas_provision_req_Activate (EasProvisionReq* self, GError** error);
void eas_provision_req_MessageComplete (EasProvisionReq* self, xmlDoc *doc, GError* error);
gchar* eas_provision_req_GetPolicyKey (EasProvisionReq* self);

G_END_DECLS

#endif /* _EAS_PROVISION_REQ_H_ */
