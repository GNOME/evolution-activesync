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
	g_print("eas_provision_msg_init++\n");
	
	object->priv = priv = EAS_PROVISION_MSG_PRIVATE(object);

	priv->policy_key = NULL;
	priv->policy_status = NULL;
	
	g_print("eas_provision_msg_init--\n");
}

static void
eas_provision_msg_finalize (GObject *object)
{
	EasProvisionMsgPrivate *priv;
	g_print("eas_provision_msg_finalize++\n");

	// g_free ignores NULL
	g_free(priv->policy_key);
	g_free(priv->policy_status);

	G_OBJECT_CLASS (eas_provision_msg_parent_class)->finalize (object);
	g_print("eas_provision_msg_finalize--\n");
}

static void
eas_provision_msg_class_init (EasProvisionMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);

	g_print("eas_provision_msg_class_init++\n");

	g_type_class_add_private (klass, sizeof (EasProvisionMsgPrivate));

	object_class->finalize = eas_provision_msg_finalize;
	
	g_print("eas_provision_msg_class_init--\n");
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
	
	g_print("eas_provision_msg_build_message++\n");

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
                             
	g_print("eas_provision_msg_build_message--\n");

    return doc;
}

void
eas_provision_msg_parse_response (EasProvisionMsg* self, xmlDoc* doc)
{
	EasProvisionMsgPrivate *priv = self->priv;
    xmlNode *node = NULL;
	gboolean found_status = FALSE, 
	         found_policy_key = FALSE;

	g_print("eas_provision_msg_parse_response++\n");

	g_free(priv->policy_key);
	g_free(priv->policy_status);

	priv->policy_key = priv->policy_status = NULL;

    if (!doc) {
        g_print ("  Failed to parse provision response XML\n");
        return;
    }
	
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "Provision")) {
        g_print("  Failed to find <Provision> element\n");
        return;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
            g_print ("  Provision Status:[%s]\n", provision_status);
            break;
        }
    }
    if (!node) {
        g_print ("  Failed to find <Status> element\n");
        return;
    }

    for (node = node->next; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policies"))
            break;
    }
    if (!node) {
        g_print("  Failed to find <Policies> element\n");
        return;
    }

    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE &&
            !strcmp((char *)node->name, "Policy"))
            break;
    }
    if (!node) {
        g_print ("  Failed to find <Policy> element\n");
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
                g_print("Policy Status:[%s]\n", priv->policy_status);
                continue;
            }
        }
        if (!found_policy_key && node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "PolicyKey"))
        {
            priv->policy_key = g_strdup(xmlNodeGetContent(node));
            if (priv->policy_key) 
            {
                found_policy_key = TRUE;
                g_print ("Provisioned PolicyKey:[%s]\n", priv->policy_key);
            }
        }

        if (found_status && found_policy_key) break;
    }
	
	g_print("eas_provision_msg_parse_response--\n");
}

EasProvisionMsg*
eas_provision_msg_new (void)
{
	EasProvisionMsg* msg = NULL;
	
	g_print("eas_provision_msg_new++\n");

	msg = g_object_new (EAS_TYPE_PROVISION_MSG, NULL);

	g_print("eas_provision_msg_new--\n");
	
	return msg;
}

gchar*
eas_provision_msg_get_policy_key (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_print("eas_provision_msg_get_policy_key+-\n");

	return priv->policy_key;
}

gchar*
eas_provision_msg_get_policy_status (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_print("eas_provision_msg_get_policy_status+-\n");
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
	
	g_print("eas_provision_msg_set_policy_key++\n");

	// g_xxx functions can handle NULL
	g_free(priv->policy_key);
	priv->policy_key = g_strdup(policyKey);
	
	g_print("eas_provision_msg_set_policy_key--\n");
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
	
	g_print("eas_provision_msg_set_policy_status++\n");

	// g_xxx functions can handle NULL
	g_free(priv->policy_status);
	priv->policy_status = g_strdup(policyStatus);
	
	g_print("eas_provision_msg_set_policy_status--\n");
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
        g_print("Failed: Unable to decode the WBXML to XML\n");
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
        g_print("Failed: Unable to decode the WBXML to XML\n");
		return;
    }

	eas_connection_parse_provision_xml_response (cnc, xml, xml_len);
	if (xml) free(xml);

	e_flag_set (priv->provision_eflag);
}


#endif