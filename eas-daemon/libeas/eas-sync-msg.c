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
	g_print("eas_sync_msg_init++\n");

	EasSyncMsgPrivate *priv;

	object->priv = priv = EAS_SYNC_MSG_PRIVATE(object);
	
	priv->sync_key = NULL;
	priv->folderID = NULL;
	priv->account_id = -1;
	g_print("eas_sync_msg_init--\n");
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
eas_sync_msg_build_message (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *node  = NULL, 
	        *child = NULL;
    xmlNs   *ns    = NULL;

	//TODO: this is taken from foldersync message - needs to be properly created
    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar*)"Sync", NULL);
    xmlDocSetRootElement (doc, node);
    
    xmlCreateIntSubset(doc, 
                       (xmlChar*)"ActiveSync", 
                       (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                       (xmlChar*)"http://www.microsoft.com/");

    ns = xmlNewNs (node, (xmlChar *)"FolderHierarchy:",NULL);
    child = xmlNewChild(node, NULL, (xmlChar *)"SyncKey", (xmlChar*)priv->sync_key);

    return doc;
}

void
eas_sync_msg_parse_reponse (EasSyncMsg* self, xmlDoc *doc)
{
	EasSyncMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	
    if (!doc) {
        g_print ("Failed to parse sync response XML\n");
        return;
    }
    node = xmlDocGetRootElement(doc);
    
    /*TODO: parse respone correctly
    *
    if (strcmp((char *)node->name, "FolderSync")) {
        g_print("Failed to find <FolderSync> element\n");
        return;
    }
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
            g_print ("FolderSync Status:[%s]\n", provision_status);
            continue;
        }
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "SyncKey")) 
        {
            priv->sync_key = g_strdup(xmlNodeGetContent(node));
            g_print ("FolderSync syncKey:[%s]\n", priv->sync_key);
            continue;
        }
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Changes")) 
		{
			break;
		}
    }
    if (!node) {
        g_print ("Failed to find Changes element\n");
        return;
    }
	
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Count"))
		{
			gchar *count = (gchar *)xmlNodeGetContent(node);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Add")) {
			eas_connection_parse_fs_add(self, node);
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
	*/
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
	EasSyncMsgPrivate *priv = self->priv;
	return priv->sync_key;
}


