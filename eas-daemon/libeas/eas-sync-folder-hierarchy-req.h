/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_
#define _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ             (eas_sync_folder_hierarchy_req_get_type ())
#define EAS_SYNC_FOLDER_HIERARCHY_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReq))
#define EAS_SYNC_FOLDER_HIERARCHY_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqClass))
#define EAS_IS_SYNC_FOLDER_HIERARCHY_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ))
#define EAS_IS_SYNC_FOLDER_HIERARCHY_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ))
#define EAS_SYNC_FOLDER_HIERARCHY_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_HIERARCHY_REQ, EasSyncFolderHierarchyReqClass))

typedef struct _EasSyncFolderHierarchyReqClass EasSyncFolderHierarchyReqClass;
typedef struct _EasSyncFolderHierarchyReq EasSyncFolderHierarchyReq;
typedef struct _EasSyncFolderHierarchyReqPrivate EasSyncFolderHierarchyReqPrivate;

struct _EasSyncFolderHierarchyReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSyncFolderHierarchyReq
{
	EasRequestBase parent_instance;

	EasSyncFolderHierarchyReqPrivate* priv;
};

GType eas_sync_folder_hierarchy_req_get_type (void) G_GNUC_CONST;

EasSyncFolderHierarchyReq* eas_sync_folder_hierarchy_req_new (const gchar* syncKey,
                                                             guint64 accountId,
                                                             EFlag *flag);

gboolean eas_sync_folder_hierarchy_req_Activate (EasSyncFolderHierarchyReq* self, GError** error);

void eas_sync_folder_hierarchy_req_MessageComplete (EasSyncFolderHierarchyReq* self, 
                                                    xmlDoc *doc, 
                                                    GError* error);

gboolean eas_sync_folder_hierarchy_req_ActivateFinish (EasSyncFolderHierarchyReq* self, 
                                                   gchar** ret_sync_key, 
                                                   GSList** added_folders, 
                                                   GSList** updated_folders, 
                                                   GSList** deleted_folders, 
                                                   GError** error);

G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_HIERARCHY_REQ_H_ */
