/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-msg.h"
#include "eas-connection-errors.h"

struct _EasProvisionMsgPrivate
{
	gchar* policy_key;
	gchar* policy_status;
};

#define EAS_PROVISION_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PROVISION_MSG, EasProvisionMsgPrivate))



G_DEFINE_TYPE (EasProvisionMsg, eas_provision_msg, EAS_TYPE_MSG_BASE);

static void
eas_provision_msg_init (EasProvisionMsg *object)
{
	EasProvisionMsgPrivate *priv;
	g_debug("eas_provision_msg_init++");
	
	object->priv = priv = EAS_PROVISION_MSG_PRIVATE(object);

	priv->policy_key = NULL;
	priv->policy_status = NULL;
	
	g_debug("eas_provision_msg_init--");
}

static void
eas_provision_msg_finalize (GObject *object)
{
	EasProvisionMsg *msg = (EasProvisionMsg *)object;
	EasProvisionMsgPrivate *priv = msg->priv;
	
	g_debug("eas_provision_msg_finalize++");

	// g_free ignores NULL
	g_free(priv->policy_key);
	g_free(priv->policy_status);

	G_OBJECT_CLASS (eas_provision_msg_parent_class)->finalize (object);
	g_debug("eas_provision_msg_finalize--");
}

static void
eas_provision_msg_class_init (EasProvisionMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);

	g_debug("eas_provision_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasProvisionMsgPrivate));

	object_class->finalize = eas_provision_msg_finalize;
	
	g_debug("eas_provision_msg_class_init--");
}


xmlDoc*
eas_provision_msg_build_message (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
    xmlDoc *doc = NULL;
    xmlDtd *dtd = NULL;
    xmlNode *node = NULL, 
	        *child = NULL, 
	        *grandchild = NULL;
    xmlNs *ns = NULL;
	
	g_debug("eas_provision_msg_build_message++");

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode(doc, NULL, (xmlChar*)"Provision", NULL);
    xmlDocSetRootElement (doc, node);

    dtd = xmlCreateIntSubset(doc, 
                    (xmlChar*)"ActiveSync", 
                    (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                    (xmlChar*)"http://www.microsoft.com/");
                    
    ns = xmlNewNs (node, (xmlChar *)"Provision:",NULL);
    xmlNewNsProp(node, ns, (xmlChar*)"xmlns:settings", (xmlChar*)"Settings:");
    child = xmlNewChild(node, ns, (xmlChar *)"Policies", NULL);
    grandchild = xmlNewChild(child, ns, (xmlChar*)"Policy", NULL);
    xmlNewChild(grandchild, ns, (xmlChar*)"PolicyType",(xmlChar*)"MS-EAS-Provisioning-WBXML");

    if (priv->policy_key) {
        xmlNewChild(grandchild, ns, (xmlChar*)"PolicyKey",(xmlChar*)priv->policy_key);
    }
    if (priv->policy_status) {
        xmlNewChild(grandchild, ns, (xmlChar*)"Status",(xmlChar*)priv->policy_status);
    }
                             
	g_debug("eas_provision_msg_build_message--");

    return doc;
}

/*
translates from eas provision status code to GError
*/
static void set_provision_status_error(guint provision_status, GError **error)
{
	switch(provision_status)
	{
		case 2:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_PROTOCOLERROR,	   
			("Provisioning error: protocol error"));	
		}
		break;
		case 3:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_GENERALSERVERERROR,	   
			("Provisioning error: general server error"));	
		}
		break;
		case 4:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_DEVICE_EXTERNALLY_MANAGED,	   
			("Provisioning error: device externally managed"));	
		}
		break;
		default:
		{
			g_warning("Unrecognised provisioning error");
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,	   
			("Unrecognised provisioning error"));			
		}
	}
}

/*
translates from eas policy status code to GError
*/
static void set_policy_status_error(guint policy_status, GError **error)
{
	switch(policy_status)
	{
		case 2:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_NOCLIENTPOLICYEXISTS,	   
			("Provisioning error: No policy for this client"));	
		}
		break;
		case 3:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_UNKNOWNPOLICYTYPE,	   
			("Provisioning error: unknown policy type value"));	
		}
		break;
		case 4:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_CORRUPTSERVERPOLICYDATA,	   
			("Provisioning error: policy data on the server is corrupted (possibly tampered with)"));	
		}
		break;
		case 5:
		{
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_ACKINGWRONGPOLICYKEY,	   
			("Provisioning error: client is acknowledging the wrong policy key"));	
		}
		default:
		{
			g_warning("Unrecognised provisioning error");
			g_set_error (error, EAS_CONNECTION_ERROR,
			EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,	   
			("unrecognised provisioning error"));			
		}			
		break;			
	}	
}

gboolean
eas_provision_msg_parse_response (EasProvisionMsg* self, xmlDoc* doc, GError** error)
{
	EasProvisionMsgPrivate *priv = self->priv;
    xmlNode *node = NULL;
	gboolean found_status = FALSE, 
	         found_policy_key = FALSE;

	g_debug("eas_provision_msg_parse_response++");

	g_free(priv->policy_key);
	g_free(priv->policy_status);

	priv->policy_key = priv->policy_status = NULL;

    if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,	   
			     ("bad argument"));		
        return FALSE;
    }
	
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "Provision")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_INVALIDXML,	   
			     ("Failed to find <Provision> element"));			
        return FALSE;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
			guint provision_status_num = atoi(provision_status);
			xmlFree(provision_status);
			if(provision_status_num != 1)  // not success
			{
				set_provision_status_error(provision_status_num, error);
				return FALSE;
			}
            break;
        }
    }
    if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_INVALIDXML,	   
			     ("Failed to find <Status> element"));					
        return FALSE;
    }

    for (node = node->next; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policies"))
            break;
    }
    if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_INVALIDXML,	   
			     ("Failed to find <Policies> element"));			
        return FALSE;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policy"))
            break;
    }
    if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_INVALIDXML,	   
			     ("Failed to find <Policy> element"));			
        return FALSE;
    }

    for (node = node->children; node; node = node->next)
    {
        if (!found_status && node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status"))
        {
			gchar *xmlTmp = xmlNodeGetContent(node);
            priv->policy_status = g_strdup(xmlTmp);
			xmlFree(xmlTmp);
            if (priv->policy_status) 
            {
                found_status = TRUE;
				guint policy_status_num = atoi(priv->policy_status);
				if(policy_status_num != 1)	// not success
				{
					set_policy_status_error(policy_status_num, error);

					return FALSE;
				}
                continue;
            }

        }
        if (!found_policy_key && node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "PolicyKey"))
        {
			gchar *xmlTmp = xmlNodeGetContent(node);
            priv->policy_key = g_strdup(xmlTmp);
			xmlFree(xmlTmp);
            if (priv->policy_key) 
            {
                found_policy_key = TRUE;
                g_debug ("Provisioned PolicyKey:[%s]", priv->policy_key);
            }
        }

        if (found_status && found_policy_key) break;
    }
	
	if(!found_status)
	{
		g_set_error (error, EAS_CONNECTION_ERROR,
				 EAS_CONNECTION_ERROR_INVALIDXML,	   
				 ("Failed to find <Status> element"));	
		return FALSE;
	}
	else if(!found_policy_key)
	{
		g_set_error (error, EAS_CONNECTION_ERROR,
				 EAS_CONNECTION_ERROR_INVALIDXML,	   
				 ("Failed to find <PolicyKey> element"));	
		return FALSE;
	}
	
	g_debug("eas_provision_msg_parse_response--");

	g_assert(*error == NULL);
	
	return TRUE;
}

EasProvisionMsg*
eas_provision_msg_new (void)
{
	EasProvisionMsg* msg = NULL;
	
	g_debug("eas_provision_msg_new++");

	msg = g_object_new (EAS_TYPE_PROVISION_MSG, NULL);

	g_debug("eas_provision_msg_new--");
	
	return msg;
}

gchar*
eas_provision_msg_get_policy_key (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_debug("eas_provision_msg_get_policy_key+-");

	return priv->policy_key;
}

gchar*
eas_provision_msg_get_policy_status (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_debug("eas_provision_msg_get_policy_status+-");
	return priv->policy_status;
}

/**
 * Setter for policy key
 * @param self the GObject
 * @param policyKey the new policy key [no transfer]
 */
void
eas_provision_msg_set_policy_key (EasProvisionMsg* self, gchar* policyKey)
{
	EasProvisionMsgPrivate *priv = self->priv;
	
	g_debug("eas_provision_msg_set_policy_key++");

	// g_xxx functions can handle NULL
	g_free(priv->policy_key);
	priv->policy_key = g_strdup(policyKey);
	
	g_debug("eas_provision_msg_set_policy_key--");
}

/**
 * Setter for policy status
 * @param self the GObject
 * @param policyKey the new policy status [no transfer]
 */
void
eas_provision_msg_set_policy_status (EasProvisionMsg* self, gchar* policyStatus)
{
	EasProvisionMsgPrivate *priv = self->priv;
	
	g_debug("eas_provision_msg_set_policy_status++");

	// g_xxx functions can handle NULL
	g_free(priv->policy_status);
	priv->policy_status = g_strdup(policyStatus);
	
	g_debug("eas_provision_msg_set_policy_status--");
}

