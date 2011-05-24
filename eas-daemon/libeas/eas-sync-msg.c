/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-msg.h"

struct _EasSyncMsgPrivate
{
	GSList* added_items;
	GSList* updated_items;
	GSList* deleted_items;

	gchar* sync_key;
	gchar* folderID;
	gint   account_id;
};

#define EAS_SYNC_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_MSG, EasSyncMsgPrivate))


G_DEFINE_TYPE (EasSyncMsg, eas_sync_msg, EAS_TYPE_MSG_BASE);

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
eas_sync_msg_new (const gchar* syncKey, gint accountId, gchar *folderID)
{
	EasSyncMsg* msg = NULL;
	EasSyncMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_MSG, NULL);
	priv = msg->priv;

	priv->sync_key = g_strdup(syncKey);
	priv->account_id = accountId;
	priv->folderID = g_strdup(folderID);

	return msg;
}

xmlDoc*
eas_sync_msg_build_message (EasSyncMsg* self, gboolean getChanges)
{
	EasSyncMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *node  = NULL, 
	        *child = NULL,
	        *grandchild = NULL;
    xmlNs   *ns    = NULL;

	//TODO: this is taken from foldersync message - needs to be properly created
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
   grandchild = xmlNewChild(child, NULL, (xmlChar *)"Collection", NULL);
   xmlNewChild(grandchild, NULL, (xmlChar *)"SyncKey", (xmlChar*)priv->sync_key);
   xmlNewChild(grandchild, NULL, (xmlChar *)"CollectionId", (xmlChar*)priv->folderID);
   if(getChanges){
   	   xmlNewChild(grandchild, NULL, (xmlChar *)"DeletesAsMoves", (xmlChar*)"1");
   	   xmlNewChild(grandchild, NULL, (xmlChar *)"GetChanges", (xmlChar*)"1");
   }
    xmlNewChild(grandchild, NULL, (xmlChar *)"WindowSize", (xmlChar*)"100");
    

    return doc;
}

void
eas_sync_msg_parse_reponse (EasSyncMsg* self, xmlDoc *doc)
{
    g_debug ("eas_sync_msg_parse_response ++");
	EasSyncMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	
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
			continue;
		}
		
	}
	
	g_debug ("eas_sync_msg_parse_response --");

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


