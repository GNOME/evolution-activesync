/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 */

#include "eas-get-email-attachment-msg.h"


struct _EasGetEmailAttachmentMsgPrivate
{
	gchar* fileReference;
	gchar* directoryPath;
};

#define EAS_GET_EMAIL_ATTACHMENT_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgPrivate))



G_DEFINE_TYPE (EasGetEmailAttachmentMsg, eas_get_email_attachment_msg, EAS_TYPE_MSG_BASE);

static void
eas_get_email_attachment_msg_init (EasGetEmailAttachmentMsg *object)
{
	EasGetEmailAttachmentMsgPrivate* priv = NULL;
	g_debug("eas_get_email_attachment_msg_init++");
		
	object->priv = priv = EAS_GET_EMAIL_ATTACHMENT_MSG_PRIVATE(object);

	priv->fileReference = NULL;
	priv->directoryPath = NULL;
	
	g_debug("eas_get_email_attachment_msg_init--");
}

static void
eas_get_email_attachment_msg_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	EasGetEmailAttachmentMsg* self = (EasGetEmailAttachmentMsg*) object;
	EasGetEmailAttachmentMsgPrivate* priv = self->priv;
	g_debug("eas_get_email_attachment_msg_finalize++");

	g_free(priv->fileReference);
	g_free(priv->directoryPath);
	
	G_OBJECT_CLASS (eas_get_email_attachment_msg_parent_class)->finalize (object);
	g_debug("eas_get_email_attachment_msg_finalize--");
}

static void
eas_get_email_attachment_msg_class_init (EasGetEmailAttachmentMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);
	g_debug("eas_get_email_attachment_msg_class_init++");
	g_type_class_add_private (klass, sizeof (EasGetEmailAttachmentMsgPrivate));

	object_class->finalize = eas_get_email_attachment_msg_finalize;
	g_debug("eas_get_email_attachment_msg_class_init++");
}

EasGetEmailAttachmentMsg*
eas_get_email_attachment_msg_new (const gchar* fileReference, const char* directoryPath)
{
	EasGetEmailAttachmentMsg* self = NULL;
	EasGetEmailAttachmentMsgPrivate* priv = NULL;
	self = g_object_new(EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, NULL);
	priv = self->priv;
	
	g_debug("eas_get_email_attachment_msg_new++");

	priv->fileReference  = g_strdup (fileReference);
	priv->directoryPath = g_strdup (directoryPath);

	g_debug("eas_get_email_attachment_msg_new--");

	return self;
}

xmlDoc*
eas_get_email_attachment_msg_build_message (EasGetEmailAttachmentMsg* self)
{

	EasGetEmailAttachmentMsgPrivate *priv = self->priv;
	xmlDoc* doc = NULL;

	xmlNode *root = NULL;
	xmlNode *fetch = NULL, 
	        *options = NULL, 
	        *body_pref = NULL, 
	        *leaf = NULL;

	g_debug("eas_get_email_attachment_msg_build_message++");

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
    leaf = xmlNewChild(fetch, NULL, (xmlChar *)"airsyncbase:FileReference",  (xmlChar*)priv->fileReference );

	g_debug("eas_get_email_attachment_msg_build_message--");
	return doc;
}

void
eas_get_email_attachment_msg_parse_response (EasGetEmailAttachmentMsg* self, xmlDoc *doc, GError** error)
{
	EasGetEmailAttachmentMsgPrivate *priv = self->priv;
	g_debug("eas_get_email_attachment_msg_parse_response ++");

	xmlNode *node = NULL;
	
    if (!doc) 
    {
        g_warning ("No XML Doc to parse");
        return;
    }
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "ItemOperations")) {
        g_debug("Failed to find <ItemOperations> element");
        return;
    }

    for (node = node->children; node; node = node->next) 
	{
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *status = (gchar *)xmlNodeGetContent(node);
            g_debug ("ItemOperations Status:[%s]", status);
			xmlFree(status);
            continue;
        }
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Response"))
		{
			break;
		}
	}
	if (!node)
	{
		g_warning("Could not find Response node");
		return;
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Fetch"))
		{
			break;
		}
	}
	if (!node)
	{
		g_warning("Could not find Fetch node");
		return;
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status"))
		{
            gchar *status = (gchar *)xmlNodeGetContent(node);
            g_debug ("Fetch Status:[%s]", status);
			xmlFree(status);
            continue;
		}
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "FileReference"))
		{
			gchar *xmlTmp = xmlNodeGetContent(node);
			priv->fileReference = g_strdup(xmlTmp);
			g_debug ("FileReference:[%s]", priv->fileReference);			
			xmlFree(xmlTmp);		
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Class"))
		{
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Properties"))
		{
			break;
		}
	}

	if (!node)
	{
		g_warning("Failed to find Properties node");
		return;
	}


	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "ContentType"))
		{
			gchar *xmlTmp = xmlNodeGetContent(node);
			g_debug ("ContentType:[%s]", xmlTmp);
			//TODO: do we need to handle the ContentType? 
			xmlFree(xmlTmp);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Data"))
		{
			gchar *xmlTmp = xmlNodeGetContent(node);
			gchar* fullFilePath = NULL;
			FILE *hAttachement = NULL;

			fullFilePath = g_strconcat(priv->directoryPath, priv->fileReference, NULL);
			g_message("Attempting to write attachment to file [%s]", fullFilePath);   
			if (hAttachement = fopen(fullFilePath,"wb"))
			{
				fputs(xmlTmp, hAttachement);
				fclose(hAttachement);
			}
			else
			{
				g_critical("Failed to open file!");
			}
			g_free(fullFilePath);
			xmlFree(xmlTmp);
			break;
		}
	}

	g_debug("eas_get_email_attachment_msg_parse_response --");
}

