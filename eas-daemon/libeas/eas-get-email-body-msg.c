/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-connection-errors.h"
#include "eas-get-email-body-msg.h"

struct _EasGetEmailBodyMsgPrivate
{
	gchar* serverUid;
	gchar* collectionId;
	gchar* directoryPath;
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
	g_free(priv->directoryPath);
	
	G_OBJECT_CLASS (eas_get_email_body_msg_parent_class)->finalize (object);
	g_debug("eas_get_email_body_msg_finalize--");
}

static void
eas_get_email_body_msg_class_init (EasGetEmailBodyMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);
	void *tmp = parent_class;
	tmp = object_class;
	
	g_debug("eas_get_email_body_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasGetEmailBodyMsgPrivate));

	object_class->finalize = eas_get_email_body_msg_finalize;
	g_debug("eas_get_email_body_msg_class_init--");
}


EasGetEmailBodyMsg*
eas_get_email_body_msg_new (const gchar* serverUid, const gchar* collectionId, const char* directoryPath)
{
	EasGetEmailBodyMsg* self = NULL;
	EasGetEmailBodyMsgPrivate* priv = NULL;
	self = g_object_new(EAS_TYPE_GET_EMAIL_BODY_MSG, NULL);
	priv = self->priv;
	
	g_debug("eas_get_email_body_msg_new++");

	priv->serverUid  = g_strdup (serverUid);
	priv->collectionId  = g_strdup (collectionId);
	priv->directoryPath = g_strdup (directoryPath);

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
    //The TruncationSize and AllOrNone doesn't work for Type MIME 4 (uncomment these for Plain text 1 or HTML 2)
    //leaf = xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:TruncationSize", (xmlChar*) "5120");  //  Set the trancation size to 5KB    
    //leaf = xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:AllOrNone", (xmlChar*) "0");

	g_debug("eas_get_email_body_msg_build_message--");
	return doc;
}

gboolean
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, xmlDoc *doc, GError** error)
{
	EasError error_details;
	gboolean ret = TRUE;
	EasGetEmailBodyMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	
	g_debug("eas_get_email_body_msg_parse_response++");

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
    if (!doc) 
    {
        g_warning ("no XML doc to parse");
		// Note not setting error here as empty doc is valid
		goto finish;
    }
    node = xmlDocGetRootElement(doc);
    if (g_strcmp0((char *)node->name, "ItemOperations")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
		EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	   
		("Failed to find <ItemOperations> element"));
        ret = FALSE;
		goto finish;
    }

    for (node = node->children; node; node = node->next) 
	{
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Status")) 
        {
            gchar *status = (gchar *)xmlNodeGetContent(node);
			EasItemOperationsStatus status_num = atoi(status);			
			xmlFree(status);
			if(status_num != EAS_COMMON_STATUS_OK)  // not success
			{
				ret = FALSE;

				// TODO - think of a nicer way to do this?
				if((EAS_CONNECTION_ERROR_INVALIDCONTENT <= status_num) && (status_num <= EAS_CONNECTION_ERROR_MAXIMUMDEVICESREACHED))// it's a common status code
				{
					error_details = common_status_error_map[status_num - 100];
				}
				else 
				{
					if(status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT)// not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = itemoperations_status_error_map[status_num];	
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
            continue;			
        }
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Response"))
		{
			break;
		}
	}
	if (!node)
	{
		g_warning("Could not find Response node");
		// Note not setting error here as this is valid	
		goto finish;		
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Fetch"))
		{
			break;
		}
	}
	if (!node)
	{
		g_warning("Could not find Fetch node");
		// Note not setting error here as this is valid	
		goto finish;		
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Status"))
		{
            gchar *status = (gchar *)xmlNodeGetContent(node);
			EasItemOperationsStatus status_num = atoi(status);			
			xmlFree(status);
			if(status_num != EAS_COMMON_STATUS_OK)  // not success
			{
				// TODO could also be a common status code!?
				if(status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT)// not pretty, but make sure we don't overrun array if new status added
				{
					status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;
				}
				error_details = itemoperations_status_error_map[status_num];
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				ret = FALSE;
				goto finish;
			}
            continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "CollectionId"))
		{
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "ServerID"))
		{
			gchar *xmlTmp = (gchar *)xmlNodeGetContent(node);
			priv->serverUid = g_strdup(xmlTmp);
			xmlFree(xmlTmp);
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Class"))
		{
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Properties"))
		{
			break;
		}
	}

	if (!node)
	{
		g_warning("Failed to find Properties node");
		// Note not setting error here as this is valid	
		goto finish;
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Body"))
		{
			break;
		}
	}

	if (!node)
	{
		g_warning("Failed to find Body node");
		// Note not setting error here as this is valid	
		goto finish;
	}

	for (node = node->children; node; node = node->next)
	{
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Type"))
		{
			gchar *xmlTmp = (gchar *)xmlNodeGetContent(node);
			if (g_strcmp0(xmlTmp,"4"))
			{
				g_critical("Email type returned by server is not MIME");
				xmlFree(xmlTmp);
				// TODO set error?
				goto finish;
			}
			xmlFree(xmlTmp);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Data"))
		{
			gchar *xmlTmp = (gchar *)xmlNodeGetContent(node);
			gchar* fullFilePath = NULL;
			FILE *hBody = NULL;

			fullFilePath = g_build_filename(priv->directoryPath, priv->serverUid, NULL);
			g_message("Attempting to write email to file [%s]",fullFilePath);   
			if ( (hBody = fopen(fullFilePath,"wb")) )
			{
				fputs(xmlTmp, hBody);
				fclose(hBody);
			}
			else
			{
				g_critical("Failed to open file!");
				// TODO set error?
			}
			g_free(fullFilePath);
			xmlFree(xmlTmp);
			break;
		}
	}

finish:
	if(!ret)
	{
		g_assert (error == NULL || *error != NULL);
	}	
	g_debug("eas_get_email_body_msg_parse_response--");	
	return ret;	

}
