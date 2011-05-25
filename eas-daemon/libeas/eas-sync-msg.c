/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-msg.h"
#include "eas-email-info-translator.h"

struct _EasSyncMsgPrivate
{
	GSList* added_items;
	GSList* updated_items;
	GSList* deleted_items;

	gchar* sync_key;
	gchar* folderID;
	gint   account_id;
	
	EasItemType ItemType;
};

#define EAS_SYNC_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_MSG, EasSyncMsgPrivate))


G_DEFINE_TYPE (EasSyncMsg, eas_sync_msg, EAS_TYPE_MSG_BASE);

static void eas_sync_parse_item_add(EasSyncMsg *self, xmlNode *node);

static void
eas_sync_msg_init (EasSyncMsg *object)
{
	g_debug("eas_sync_msg_init++");

	EasSyncMsgPrivate *priv;

	object->priv = priv = EAS_SYNC_MSG_PRIVATE(object);
	
	priv->sync_key = NULL;
	priv->folderID = NULL;
	priv->account_id = -1;
	g_debug("eas_sync_msg_init--");
}

static void
eas_sync_msg_finalize (GObject *object)
{
	EasSyncMsg *msg = (EasSyncMsg *)object;
	EasSyncMsgPrivate *priv = msg->priv;

	g_free(priv->sync_key);
	g_free(priv->folderID);
	
	G_OBJECT_CLASS (eas_sync_msg_parent_class)->finalize (object);
}

static void
eas_sync_msg_class_init (EasSyncMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (EasSyncMsgPrivate));

	object_class->finalize = eas_sync_msg_finalize;
}

EasSyncMsg*
eas_sync_msg_new (const gchar* syncKey, gint accountId, gchar *folderID, EasItemType type)
{
	EasSyncMsg* msg = NULL;
	EasSyncMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_MSG, NULL);
	priv = msg->priv;

	priv->sync_key = g_strdup(syncKey);
	priv->account_id = accountId;
	priv->folderID = g_strdup(folderID);
	priv->ItemType = type;

	return msg;
}

xmlDoc*
eas_sync_msg_build_message (EasSyncMsg* self, gboolean getChanges)
{
	EasSyncMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *node  = NULL, 
	        *child = NULL,
	        *collection = NULL,
	        *options = NULL,
	        *body_pref = NULL;
    xmlNs   *ns    = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar*)"Sync", NULL);
    xmlDocSetRootElement (doc, node);
    
    xmlCreateIntSubset(doc, 
                       (xmlChar*)"ActiveSync", 
                       (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                       (xmlChar*)"http://www.microsoft.com/");

	ns = xmlNewNs (node, (xmlChar *)"AirSync:",NULL);
	xmlNewNs (node, (xmlChar *)"AirSyncBase:", (xmlChar *)"airsyncbase");
	child = xmlNewChild(node, NULL, (xmlChar *)"Collections", NULL);
	collection = xmlNewChild(child, NULL, (xmlChar *)"Collection", NULL);
	xmlNewChild(collection, NULL, (xmlChar *)"SyncKey", (xmlChar*)priv->sync_key);
	xmlNewChild(collection, NULL, (xmlChar *)"CollectionId", (xmlChar*)priv->folderID);
	if(getChanges){
		xmlNewChild(collection, NULL, (xmlChar *)"DeletesAsMoves", (xmlChar*)"1");
		xmlNewChild(collection, NULL, (xmlChar *)"GetChanges", (xmlChar*)"1");
		xmlNewChild(collection, NULL, (xmlChar *)"WindowSize", (xmlChar*)"100");
   
		if(priv->ItemType == EAS_ITEM_MAIL){
        
        options = xmlNewChild(collection, NULL, (xmlChar *)"Options", NULL);
            xmlNewChild(options, NULL, (xmlChar *)"FilterType", (xmlChar*)"0");
            xmlNewChild(options, NULL, (xmlChar *)"MIMESupport", (xmlChar*)"2");
            xmlNewChild(options, NULL, (xmlChar *)"MIMETruncation", (xmlChar*)"0");

            body_pref = xmlNewChild(options, NULL, (xmlChar *)"airsyncbase:BodyPreference", NULL);
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:Type", (xmlChar*)"4"); // Plain text 1, HTML 2, MIME 4
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:TruncationSize", (xmlChar*)"200000");
		}
    }    

    return doc;
}

void
eas_sync_msg_parse_reponse (EasSyncMsg* self, xmlDoc *doc)
{
    g_debug ("eas_sync_msg_parse_response ++");
	EasSyncMsgPrivate *priv = self->priv;
	xmlNode *node = NULL,
					*appData = NULL;
					
	gchar *item_server_id = NULL;
	
    if (!doc) {
        g_debug ("Failed to parse sync response XML");
        return;
    }
    node = xmlDocGetRootElement(doc);
    
    //TODO: parse response correctly
    
    if (strcmp((char *)node->name, "Sync")) {
        g_debug("Failed to find <Sync> element");
        return;
    }
    for (node = node->children; node; node = node->next) {
    
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Collections")) 
        {
               g_debug ("Collections:");
               break;
        }

    }
    if (!node) {
        g_debug ("Failed to find Collections element");
        return;
    }
    
    for (node = node->children; node; node = node->next) {
    
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Collection")) 
        {
               g_debug ("Collection:");
               break;
        }

    }
    if (!node) {
        g_debug ("Failed to find Collection element");
        return;
    }
	
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "SyncKey"))
		{
			priv->sync_key = (gchar *)xmlNodeGetContent(node);
			g_debug ("Got SyncKey = %s", priv->sync_key);
			continue;
		}		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Commands")) 
        {
               g_debug ("Commands:\n");
               break;
        }		
	}

    if (!node) {
        g_debug ("Failed to find Commands element\n");
        return;
    }
    
     for (node = node->children; node; node = node->next) {
	
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Add")) {
			appData = node;
			
			for (appData = appData->children; appData; appData = appData->next) {
				if (appData->type == XML_ELEMENT_NODE && !strcmp((char *)appData->name, "ServerId")) {
					item_server_id = (gchar *)xmlNodeGetContent(appData);
					g_debug ("Found serverID for Item = %s", item_server_id);
					continue;
				}
				if (appData->type == XML_ELEMENT_NODE && !strcmp((char *)appData->name, "ApplicationData")) {
					gchar *flatItem = NULL;
					g_debug ("Found AppliicationData - about to parse and flatten");
					//TODO: switch and add other translators
					flatItem = eas_add_email_appdata_parse_response(appData, item_server_id); 
					g_debug ("FlatItem = %s", flatItem);
					if(flatItem){
						g_slist_append(priv->added_items, flatItem);
					}
						
					continue;
				}
			}
			eas_sync_parse_item_add(self, node);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Delete")) {
			// TODO Parse deleted folders
			g_assert(0);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Update")) {
			// TODO Parse updated folders
			g_assert(0);
			continue;
		}
	}
	
	g_debug ("eas_sync_msg_parse_response --");

}

static void
eas_sync_parse_item_add(EasSyncMsg *self, xmlNode *node) 
{
	EasSyncMsgPrivate *priv = self->priv;

}

GSList*
eas_sync_msg_get_added_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->added_items;
}

GSList*
eas_sync_msg_get_updated_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->updated_items;
}

GSList*
eas_sync_msg_get_deleted_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->deleted_items;
}

gchar* 
eas_sync_msg_get_syncKey(EasSyncMsg* self)
{
    g_debug ("eas_sync_msg_getSyncKey ++");
	EasSyncMsgPrivate *priv = self->priv;
	return priv->sync_key;
}


