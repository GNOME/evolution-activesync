/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * 
 */

#include "eas-sync-folder-hierarchy.h"
#include "eas-sync-folder-msg.h"



G_DEFINE_TYPE (EasSyncFolderHierarchy, eas_sync_folder_hierarchy, EAS_TYPE_REQUEST_BASE);

#define EAS_SYNC_FOLDER_HIERARCHY_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_FOLDER_HIERARCHY, EasSyncFolderHierarchyPrivate))


struct _EasSyncFolderHierarchyPrivate {
	EasSyncFolderMsg* syncFolderMsg;
};

static void
eas_sync_folder_hierarchy_init (EasSyncFolderHierarchy *object)
{
	EasSyncFolderHierarchyPrivate *priv;

	object->priv = priv = EAS_SYNC_FOLDER_HIERARCHY_PRIVATE(object);

	priv->syncFolderMsg = NULL;
}

static void
eas_sync_folder_hierarchy_finalize (GObject *object)
{
	EasSyncFolderHierarchy *req = (EasSyncFolderHierarchy *) object;
	EasSyncFolderHierarchyPrivate *priv = req->priv;
	/* TODO: Add deinitalization code here */

	if (priv->syncFolderMsg) {
		g_object_unref(priv->syncFolderMsg);
	}

	G_OBJECT_CLASS (eas_sync_folder_hierarchy_parent_class)->finalize (object);
}

static void
eas_sync_folder_hierarchy_class_init (EasSyncFolderHierarchyClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasSyncFolderHierarchyPrivate));
	object_class->finalize = eas_sync_folder_hierarchy_finalize;
}


void
eas_sync_folder_hierarchy_Activate (EasSyncFolderHierarchy* self, const gchar* syncKey, guint64 accountId, EFlag *flag)
{
	EasSyncFolderHierarchyPrivate* priv = self->priv;
	xmlDoc *doc;

	eas_request_base_SetFlag(&self->parent_instance, flag);
	
	//create syn folder msg type
	priv->syncFolderMsg = eas_sync_folder_msg_new (syncKey, accountId);

	//build request msg
	doc = eas_sync_folder_msg_build_message (priv->syncFolderMsg);

	// TODO
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "FolderSync", doc, self);
}

/**
 * @param doc The protocol xml to be parsed. MUST be freed with xmlFree()
 */
void 
eas_sync_folder_hierarchy_MessageComplete (EasSyncFolderHierarchy* self, xmlDoc* doc)
{
	EasSyncFolderHierarchyPrivate *priv = self->priv;

	eas_sync_folder_msg_parse_reponse (priv->syncFolderMsg, doc);

	xmlFree(doc);
	/* TODO: Add public function implementation here */

	// Decide if to send another message, or if to inform the daemon this
	// request is now complete.
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));
}

void eas_sync_folder_hierarchy_Activate_Finish (EasSyncFolderHierarchy* self,
                                                gchar** ret_sync_key,
												GSList** added_folders,
												GSList** updated_folders,
												GSList** deleted_folders)
{
}
