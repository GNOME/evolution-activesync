/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-folder-msg.h"
#include "../../libeasmail/src/eas-folder.h"

struct _EasSyncFolderMsgPrivate
{
	GSList* added_folders;
	GSList* updated_folders;
	GSList* deleted_folders;

	gchar* sync_key;
	gint   account_id;
};

#define EAS_SYNC_FOLDER_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgPrivate))



G_DEFINE_TYPE (EasSyncFolderMsg, eas_sync_folder_msg, EAS_TYPE_MSG_BASE);

static void eas_connection_parse_fs_add(EasSyncFolderMsg *self, xmlNode *node);


static void
eas_sync_folder_msg_init (EasSyncFolderMsg *object)
{
	g_debug("eas_sync_folder_msg_init++");

	EasSyncFolderMsgPrivate *priv;

	object->priv = priv = EAS_SYNC_FOLDER_MSG_PRIVATE(object);
	
	priv->sync_key = NULL;
	priv->account_id = -1;
	g_debug("eas_sync_folder_msg_init--");

}

static void
eas_sync_folder_msg_finalize (GObject *object)
{
	EasSyncFolderMsg *msg = (EasSyncFolderMsg *)object;
	EasSyncFolderMsgPrivate *priv = msg->priv;

	g_free(priv->sync_key);
	
	G_OBJECT_CLASS (eas_sync_folder_msg_parent_class)->finalize (object);
}

static void
eas_sync_folder_msg_class_init (EasSyncFolderMsgClass *klass)
{
	g_debug("eas_sync_folder_msg_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSyncFolderMsgPrivate));

	object_class->finalize = eas_sync_folder_msg_finalize;
	g_debug("eas_sync_folder_msg_class_init--");

}


EasSyncFolderMsg*
eas_sync_folder_msg_new (const gchar* syncKey, gint accountId)
{
	EasSyncFolderMsg* msg = NULL;
	EasSyncFolderMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_FOLDER_MSG, NULL);
	priv = msg->priv;

	priv->sync_key = g_strdup(syncKey);
	priv->account_id = accountId;

	return msg;
}

xmlDoc*
eas_sync_folder_msg_build_message (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *node  = NULL, 
	        *child = NULL;
    xmlNs   *ns    = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar*)"FolderSync", NULL);
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
eas_sync_folder_msg_parse_reponse (EasSyncFolderMsg* self, xmlDoc *doc, GError** error)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	
    if (!doc) {
        g_debug ("Failed to parse folder_sync response XML");
        return;
    }
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "FolderSync")) {
        g_debug("Failed to find <FolderSync> element");
        return;
    }
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
            g_debug ("FolderSync Status:[%s]", provision_status);
			xmlFree(provision_status);
            continue;
        }
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "SyncKey")) 
        {
            priv->sync_key = xmlNodeGetContent(node);
            g_debug ("FolderSync syncKey:[%s]", priv->sync_key);
            continue;
        }
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Changes")) 
		{
			break;
		}
    }
    if (!node) {
        g_debug ("Failed to find Changes element");
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
	
}

static void
eas_connection_parse_fs_add(EasSyncFolderMsg *self, xmlNode *node) 
{
	EasSyncFolderMsgPrivate *priv = self->priv;

	if (!node) return;
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Add")) 
	{
		xmlNode *n = node;
		gchar *serverId = NULL, 
		      *parentId = NULL,
			  *displayName = NULL,
			  *type = NULL;
		
		for (n = n->children; n; n = n->next) {
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "ServerId")) {
				serverId = (gchar *)xmlNodeGetContent(n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "ParentId")) {
				parentId = (gchar *)xmlNodeGetContent(n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "DisplayName")) {
				displayName = (gchar *)xmlNodeGetContent(n);
				continue;
			}
			if (n->type == XML_ELEMENT_NODE && !strcmp((char *)n->name, "Type")) {
				type = (gchar *)xmlNodeGetContent(n);
				continue;
			}
		}
		
		if (serverId && parentId && displayName && type) 
		{
			EasFolder *f = NULL;

			f = eas_folder_new ();
			
			// Memory ownership given to EasFolder
			f->parent_id = g_strdup (parentId);
			f->folder_id = g_strdup (serverId);
			f->display_name = g_strdup (displayName);
			f->type = atoi(type);

			priv->added_folders = g_slist_append(priv->added_folders, f);
		}
		else 
		{
			g_debug("Failed to parse folderSync Add");
		}

		xmlFree(parentId);
		xmlFree(serverId);
		xmlFree(displayName);
		xmlFree(type);
	}
}


GSList*
eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->added_folders;
}

GSList*
eas_sync_folder_msg_get_updated_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->updated_folders;
}

GSList*
eas_sync_folder_msg_get_deleted_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->deleted_folders;
}

gchar* 
eas_sync_folder_msg_get_syncKey(EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	return priv->sync_key;
}

