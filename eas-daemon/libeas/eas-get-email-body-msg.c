/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-get-email-body-msg.h"

struct _EasGetEmailBodyMsgPrivate
{
	gchar* serverUid;
	gchar* collectionId;
};

#define EAS_GET_EMAIL_BODY_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgPrivate))



G_DEFINE_TYPE (EasGetEmailBodyMsg, eas_get_email_body_msg, EAS_TYPE_MSG_BASE);

static void
eas_get_email_body_msg_init (EasGetEmailBodyMsg *object)
{
	EasGetEmailBodyMsgPrivate* priv;
	g_debug("eas_get_email_body_msg_init++");

	object->priv = priv = EAS_GET_EMAIL_BODY_MSG_PRIVATE(object);

	priv->serverUid = NULL;
	priv->collectionId = NULL;
	
	g_debug("eas_get_email_body_msg_init--");
}

static void
eas_get_email_body_msg_finalize (GObject *object)
{
	EasGetEmailBodyMsg* self = (EasGetEmailBodyMsg *)object;
	EasGetEmailBodyMsgPrivate* priv = self->priv;
	/* TODO: Add deinitalization code here */
	g_debug("eas_get_email_body_msg_finalize++");

	g_free(priv->serverUid);
	g_free(priv->collectionId);
	
	G_OBJECT_CLASS (eas_get_email_body_msg_parent_class)->finalize (object);
	g_debug("eas_get_email_body_msg_finalize--");
}

static void
eas_get_email_body_msg_class_init (EasGetEmailBodyMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);
	g_debug("eas_get_email_body_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyMsgPrivate));

	object_class->finalize = eas_get_email_body_msg_finalize;
	g_debug("eas_get_email_body_msg_class_init--");
}


EasGetEmailBodyMsg*
eas_get_email_body_msg_new (const gchar* serverUid, const gchar* collectionId)
{
	EasGetEmailBodyMsg* self = NULL;
	EasGetEmailBodyMsgPrivate* priv = NULL;
	self = g_object_new(EAS_TYPE_GET_EMAIL_BODY_MSG, NULL);
	priv = self->priv;
	
	g_debug("eas_get_email_body_msg_new++");

	priv->serverUid  = g_strdup (serverUid);
	priv->collectionId  = g_strdup (collectionId);

	g_debug("eas_get_email_body_msg_new--");

	return self;
}

xmlDoc*
eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self)
{
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	xmlDoc* doc = NULL;
	xmlNode *root = NULL;
	xmlNode *fetch = NULL, 
	        *options = NULL, 
	        *body_pref = NULL, 
	        *leaf = NULL;
	
	g_debug("eas_get_email_body_msg_build_message++");

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar*)"ItemOperations", NULL);
	xmlDocSetRootElement (doc, root);
	
	xmlCreateIntSubset(doc, 
	                   (xmlChar*)"ActiveSync", 
	                   (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
	                   (xmlChar*)"http://www.microsoft.com/");
	
	xmlNewNs (root, (xmlChar *)"ItemOperations:", NULL);
	xmlNewNs (root, (xmlChar *)"AirSync:",(xmlChar *)"airsync");
	xmlNewNs (root, (xmlChar *)"AirSyncBase:", (xmlChar *)"airsyncbase");

	fetch = xmlNewChild(root, NULL, (xmlChar *)"Fetch", NULL);
	
    leaf = xmlNewChild(fetch, NULL, (xmlChar *)"Store", (xmlChar*)"Mailbox");
    leaf = xmlNewChild(fetch, NULL, (xmlChar *)"airsync:CollectionId",  (xmlChar*)priv->collectionId); 
    leaf = xmlNewChild(fetch, NULL, (xmlChar *)"airsync:ServerId",  (xmlChar*)priv->serverUid);
    options = xmlNewChild(fetch, NULL, (xmlChar *)"Options", NULL);
    
    leaf = xmlNewChild(options, NULL, (xmlChar *)"airsync:MIMESupport", (xmlChar*)"2"); // gives a protocol error in 12.1   
    body_pref = xmlNewChild(options, NULL, (xmlChar *)"airsyncbase:BodyPreference", NULL);
    
    leaf = xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:Type", (xmlChar*)"4");  // Plain text 1, HTML 2, MIME 4

	g_debug("eas_get_email_body_msg_build_message--");
	return doc;
}

void
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, xmlDoc *doc)
{
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	g_debug("eas_get_email_body_msg_parse_response++");
	/* TODO: Add public function implementation here */

	xmlNode *node = NULL;
	
    if (!doc) 
    {
        g_warning ("No XML Doc to parse");
        return;
    }
    node = xmlDocGetRootElement(doc);

	
	g_debug("eas_get_email_body_msg_parse_response--");
}
