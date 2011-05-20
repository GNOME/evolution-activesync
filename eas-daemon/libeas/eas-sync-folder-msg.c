/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-sync-folder-msg.h"

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

static void
eas_sync_folder_msg_init (EasSyncFolderMsg *object)
{
	EasSyncFolderMsgPrivate *priv;

	object->priv = priv = EAS_SYNC_FOLDER_MSG_PRIVATE(object);
	
	priv->sync_key = NULL;
	priv->account_id = -1;
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
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSyncFolderMsgPrivate));

	object_class->finalize = eas_sync_folder_msg_finalize;
}


EasSyncFolderMsg*
eas_sync_folder_msg_new (gchar* syncKey, gint accountId)
{
	EasSyncFolderMsg* msg = NULL;
	EasSyncFolderMsgPrivate *priv = NULL;
	/* TODO: Add public function implementation here */

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
eas_sync_folder_msg_parse_reponse (EasSyncFolderMsg* self, xmlDoc *doc)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	xmlNode *node = NULL;
	
    if (!doc) {
        printf ("Failed to parse folder_sync response XML\n");
        return;
    }
    node = xmlDocGetRootElement(doc);
    if (strcmp((char *)node->name, "FolderSync")) {
        printf("Failed to find <FolderSync> element\n");
        return;
    }
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Status")) 
        {
            gchar *provision_status = (gchar *)xmlNodeGetContent(node);
            printf ("FolderSync Status:[%s]\n", provision_status);
            continue;
        }
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "SyncKey")) 
        {
            priv->sync_key = g_strdup(xmlNodeGetContent(node));
            printf ("FolderSync syncKey:[%s]\n", priv->sync_key);
            continue;
        }
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Changes")) 
		{
			break;
		}
    }
    if (!node) {
        printf ("Failed to find Changes element\n");
        return;
    }
	
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Count"))
		{
			gchar *count = (gchar *)xmlNodeGetContent(node);
			continue;
		}
		
		if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, "Add")) {
			// eas_connection_parse_fs_add(cnc, node);
			/* TODO: Add public function implementation here */
			continue;
		}
	}
	
}

GSList*
eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self)
{
	EasSyncFolderMsgPrivate *priv = self->priv;
	/* TODO: Add public function implementation here */
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

#if 0
static void
eas_connection_parse_fs_add(EasSyncFolderMsg *cnc, xmlNode *node) 
{
	EasSyncFolderMsgPrivate *priv = cnc->priv;

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
		if (serverId && parentId && displayName && type) {
			EasFolderListItem *item = NULL, *leaf = NULL;
			EasFolder *f = NULL;

			item = g_malloc0(sizeof(EasFolderListItem));

			if (!item) {
				return;
			}
			                 
//			f = eas_folder_new ();
			if (!f) {
				g_free (item);
				return;
			}
			
			// Memory ownership given to EasFolder
			f->parent_id = g_strdup (parentId);
			f->folder_id = g_strdup (serverId);
			f->display_name = g_strdup (displayName);
			f->type = atoi(type);

			item->folder = f;
			item->next = NULL;

			// Move to the end of the linked list
			for (leaf = priv->created_folder_head; leaf; leaf = leaf->next);

			leaf = item;
		}
		else {
			printf("Failed to parse folderSync Add\n");
		}
	}
}
#endif