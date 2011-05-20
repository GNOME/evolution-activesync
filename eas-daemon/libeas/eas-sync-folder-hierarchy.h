/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * 
 */

#ifndef _EAS_SYNC_FOLDER_HIERARCHY_H_
#define _EAS_SYNC_FOLDER_HIERARCHY_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_HIERARCHY             (eas_sync_folder_hierarchy_get_type ())
#define EAS_SYNC_FOLDER_HIERARCHY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY, EasSyncFolderHierarchy))
#define EAS_SYNC_FOLDER_HIERARCHY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY, EasSyncFolderHierarchyClass))
#define EAS_IS_SYNC_FOLDER_HIERARCHY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY))
#define EAS_IS_SYNC_FOLDER_HIERARCHY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY))
#define EAS_SYNC_FOLDER_HIERARCHY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY, EasSyncFolderHierarchyClass))

typedef struct _EasSyncFolderHierarchyClass EasSyncFolderHierarchyClass;
typedef struct _EasSyncFolderHierarchy EasSyncFolderHierarchy;
typedef struct _EasSyncFolderHierarchyPrivate EasSyncFolderHierarchyPrivate;

struct _EasSyncFolderHierarchyClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSyncFolderHierarchy
{
	EasRequestBase parent_instance;

	EasSyncFolderHierarchyPrivate *priv;
};

GType eas_sync_folder_hierarchy_get_type (void) G_GNUC_CONST;

void eas_sync_folder_hierarchy_Activate (EasSyncFolderHierarchy *self, const gchar* syncKey, guint64 accountId, EFlag *flag);
void eas_sync_folder_hierarchy_MessageComplete (EasSyncFolderHierarchy *self, xmlDoc* doc);
void eas_sync_folder_hierarchy_Activate_Finish (EasSyncFolderHierarchy* self, 
                                                gchar** ret_sync_key, 
                                                GSList** added_folders, 
                                                GSList** updated_folders, 
                                                GSList** deleted_folders);

G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_HIERARCHY_H_ */
