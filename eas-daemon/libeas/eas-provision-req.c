/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-req.h"
#include "eas-provision-msg.h"
#include "eas-connection-errors.h"

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

	eas_request_base_SetRequestType (&object->parent_instance, EAS_REQ_PROVISION);
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

/**
 * @return whether successful
*/
gboolean
eas_provision_req_Activate (EasProvisionReq* self, GError** error)
{
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	gboolean ret = FALSE;
	EasProvisionReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;

	doc = eas_provision_msg_build_message (priv->msg);
	if(!doc)
	{
		// set the error
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     ("out of memory"));		
		goto finish;
	}

	// TODO
	ret = eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), 
	                            "Provision", 
	                            doc, 
	                            self,
	                            error);
finish:
	return ret;
}

/**
 * @param doc The protocol xml to be parsed. MUST be freed with xmlFree()
 * @return whether successful
 */
gboolean
eas_provision_req_MessageComplete (EasProvisionReq* self, xmlDoc *doc, GError** error)
{
	gboolean ret;
	EasProvisionReqPrivate *priv = self->priv;

	g_debug("eas_provision_req_MessageComplete++");
	
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	ret = eas_provision_msg_parse_response (priv->msg, doc, error);
	if(!ret)
	{
		goto finish;
	}

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
			
			g_debug("eas_provision_req_MessageComplete - EasProvisionStep1");
			
			msg = eas_provision_msg_new (); //TODO check return
			eas_provision_msg_set_policy_status (msg, eas_provision_msg_get_policy_status (priv->msg));
			eas_provision_msg_set_policy_key (msg, eas_provision_msg_get_policy_key (priv->msg));

			eas_connection_set_policy_key(eas_request_base_GetConnection (&self->parent_instance), 
			                              eas_provision_msg_get_policy_key (priv->msg));

			g_object_unref(priv->msg);
			priv->msg = msg;

			priv->state = EasProvisionStep2;

			ret = eas_provision_req_Activate (self, error);			
		}
		break;

		// We no have the final provisioning policy key
		// Set the policy key in the connection, allowing the original request
		// from the daemon that triggered the provisioning to proceed.
		case EasProvisionStep2:
		{
		g_debug("eas_provision_req_MessageComplete - EasProvisionStep2");

		 eas_connection_set_policy_key(eas_request_base_GetConnection (&self->parent_instance), 
		                               eas_provision_msg_get_policy_key (priv->msg));
		 eas_connection_resume_request(eas_request_base_GetConnection (&self->parent_instance));
		}
		break;
	}

finish:
	if(!ret)
	{
		g_assert (error == NULL || *error != NULL);
	}	
	xmlFree(doc);
	g_debug("eas_provision_req_MessageComplete--");

	return ret;
}

gchar*
eas_provision_req_GetPolicyKey (EasProvisionReq* self)
{
	/* TODO: Add public function implementation here */
}
