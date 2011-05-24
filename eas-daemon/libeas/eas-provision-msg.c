/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-msg.h"

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

void
eas_provision_msg_parse_response (EasProvisionMsg* self, xmlDoc* doc)
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
        g_debug ("  Failed to parse provision response XML");
        return;
    }
	
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "Provision")) {
        g_debug("  Failed to find <Provision> element");
        return;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
            g_debug ("  Provision Status:[%s]", provision_status);
            break;
        }
    }
    if (!node) {
        g_debug ("  Failed to find <Status> element");
        return;
    }

    for (node = node->next; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policies"))
            break;
    }
    if (!node) {
        g_debug("  Failed to find <Policies> element");
        return;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policy"))
            break;
    }
    if (!node) {
        g_debug ("  Failed to find <Policy> element");
        return;
    }

    for (node = node->children; node; node = node->next)
    {
        if (!found_status && node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status"))
        {
            priv->policy_status = g_strdup(xmlNodeGetContent(node));
            if (priv->policy_status) 
            {
                found_status = TRUE;
                g_debug("Policy Status:[%s]", priv->policy_status);
                continue;
            }
        }
        if (!found_policy_key && node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "PolicyKey"))
        {
            priv->policy_key = g_strdup(xmlNodeGetContent(node));
            if (priv->policy_key) 
            {
                found_policy_key = TRUE;
                g_debug ("Provisioned PolicyKey:[%s]", priv->policy_key);
            }
        }

        if (found_status && found_policy_key) break;
    }
	
	g_debug("eas_provision_msg_parse_response--");
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

#if 0

static void
handle_provision_stage1(SoupSession *session, SoupMessage *msg, gpointer data)
{
	EasConnection *cnc = (EasConnection *)data;
	EasConnectionPrivate *priv = cnc->priv;

    xmlDoc *doc = NULL;
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;

	if (FALSE == isResponseValid(msg)) {
		return;
	}

    wbxml2xml ((WB_UTINY*)msg->response_body->data,
               msg->response_body->length,
               &xml,
               &xml_len);

    if (!xml) 
    {
        g_debug("Failed: Unable to decode the WBXML to XML");
		return;
    }

	eas_connection_parse_provision_xml_response (cnc, xml, xml_len);
	if (xml) free(xml);

	doc = build_provision_as_xml(priv->policy_key, priv->policy_status);

	eas_connection_send_msg(cnc, 
	                        "Provision", 
	                        priv->device_id, 
	                        priv->device_type, 
	                        doc, 
	                        handle_provision_stage2, 
	                        cnc);
}

static void
handle_provision_stage2(SoupSession *session, SoupMessage *msg, gpointer data)
{
	EasConnection *cnc = (EasConnection *)data;
	EasConnectionPrivate *priv = cnc->priv;

    xmlDoc *doc = NULL;
    WB_UTINY *xml = NULL;
    WB_ULONG xml_len = 0;

	if (FALSE == isResponseValid(msg)) {
		return;
	}

    wbxml2xml ((WB_UTINY*)msg->response_body->data,
               msg->response_body->length,
               &xml,
               &xml_len);

    if (!xml) 
    {
        g_debug("Failed: Unable to decode the WBXML to XML");
		return;
    }

	eas_connection_parse_provision_xml_response (cnc, xml, xml_len);
	if (xml) free(xml);

	e_flag_set (priv->provision_eflag);
}


#endif