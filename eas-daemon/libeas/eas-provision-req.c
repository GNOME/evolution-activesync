/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-req.h"
#include "eas-provision-msg.h"

struct _EasProvisionReqPrivate
{
	EasProvisionMsg* msg;
};

#define EAS_PROVISION_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PROVISION_REQ, EasProvisionReqPrivate))



G_DEFINE_TYPE (EasProvisionReq, eas_provision_req, EAS_TYPE_REQUEST_BASE);


static void
eas_provision_req_init (EasProvisionReq *object)
{
	EasProvisionReqPrivate *priv;

	object->priv = priv = EAS_PROVISION_REQ_PRIVATE(object);

	priv->msg = NULL;
}

static void
eas_provision_req_finalize (GObject *object)
{
	EasProvisionReq *req = (EasProvisionReq *) object;
	EasProvisionReqPrivate *priv = req->priv;
	
	/* TODO: Add deinitalization code here */
	if (priv->msg) {
		g_object_unref(priv->msg);
	}

	G_OBJECT_CLASS (eas_provision_req_parent_class)->finalize (object);
}

static void
eas_provision_req_class_init (EasProvisionReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasProvisionReqPrivate));

	object_class->finalize = eas_provision_req_finalize;
}


EasProvisionReq*
eas_provision_req_new (gchar* policy_status, gchar* policy_key)
{
	/* TODO: Add public function implementation here */
}

void
eas_provision_req_Activate (EasProvisionReq* self)
{
	/* TODO: Add public function implementation here */
}

void
eas_provision_req_MessageComplete (EasProvisionReq* self, xmlDoc *doc)
{
	/* TODO: Add public function implementation here */
}

gchar*
eas_provision_req_GetPolicyKey (EasProvisionReq* self)
{
	/* TODO: Add public function implementation here */
}
