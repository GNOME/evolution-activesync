/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-req.h"
#include "eas-provision-msg.h"

typedef enum {
	EasProvisionStep1 = 0,
	EasProvisionStep2
} EasProvisionState;

struct _EasProvisionReqPrivate
{
	EasProvisionMsg* msg;
	EasProvisionState state;
	
};

#define EAS_PROVISION_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PROVISION_REQ, EasProvisionReqPrivate))



G_DEFINE_TYPE (EasProvisionReq, eas_provision_req, EAS_TYPE_REQUEST_BASE);


static void
eas_provision_req_init (EasProvisionReq *object)
{
	EasProvisionReqPrivate *priv;

	object->priv = priv = EAS_PROVISION_REQ_PRIVATE(object);

	priv->msg = NULL;
	priv->state = EasProvisionStep1;
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
	EasProvisionReqPrivate *priv = NULL;
	EasProvisionReq* req = NULL;

	req = g_object_new(EAS_TYPE_PROVISION_REQ, NULL);
	priv = req->priv;

	// Build the message
	priv->msg = eas_provision_msg_new ();
	eas_provision_msg_set_policy_status (priv->msg, policy_status);
	eas_provision_msg_set_policy_key (priv->msg, policy_key);

	return req;
}

void
eas_provision_req_Activate (EasProvisionReq* self)
{
	EasProvisionReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;

	doc = eas_provision_msg_build_message (priv->msg);

	// TODO
//	eas_connection_send_request(priv->cnc, doc, self);
	xmlFree(doc);
}

/**
 * @param doc The protocol xml to be parsed. MUST be freed with xmlFree()
 */
void
eas_provision_req_MessageComplete (EasProvisionReq* self, xmlDoc *doc)
{
	EasProvisionReqPrivate *priv = self->priv;

	eas_provision_msg_parse_response (priv->msg, doc);

	switch (priv->state) {
		default:
		{
			g_assert(0);
		}
		break;

		// We are receiving the temporary policy key and need to now make a 
		// second provision msg and send it using the new data.
		case EasProvisionStep1:
		{
			EasProvisionMsg *msg = NULL;
			
			msg = eas_provision_msg_new ();
			eas_provision_msg_set_policy_status (msg, eas_provision_msg_get_policy_status (priv->msg));
			eas_provision_msg_set_policy_key (msg, eas_provision_msg_get_policy_key (priv->msg));

//			eas_connection_set_policy_key(cnc, eas_provision_msg_get_policy_key (priv->msg));

			g_object_unref(priv->msg);
			priv->msg = msg;

			priv->state = EasProvisionStep2;

			eas_provision_req_Activate (self);
		}
		break;

		// We no have the final provisioning policy key
		// Set the policy key in the connection, allowing the original request
		// from the daemon that triggered the provisioning to proceed.
		case EasProvisionStep2:
		{
		//	eas_connection_set_policy_key(cnc, eas_provision_msg_get_policy_key (priv->msg));
		// eas_continue_original_request(cnc);
		}
		break;
	}

	xmlFree(doc);
}

gchar*
eas_provision_req_GetPolicyKey (EasProvisionReq* self)
{
	/* TODO: Add public function implementation here */
}
