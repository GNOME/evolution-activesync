/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_SYNC_FOLDER_MSG_H_
#define _EAS_SYNC_FOLDER_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_MSG             (eas_sync_folder_msg_get_type ())
#define EAS_SYNC_FOLDER_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsg))
#define EAS_SYNC_FOLDER_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))
#define EAS_IS_SYNC_FOLDER_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_IS_SYNC_FOLDER_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_SYNC_FOLDER_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))

typedef struct _EasSyncFolderMsgClass EasSyncFolderMsgClass;
typedef struct _EasSyncFolderMsg EasSyncFolderMsg;
typedef struct _EasSyncFolderMsgPrivate EasSyncFolderMsgPrivate;

struct _EasSyncFolderMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasSyncFolderMsg
{
	EasMsgBase parent_instance;

	EasSyncFolderMsgPrivate *priv;
};

GType eas_sync_folder_msg_get_type (void) G_GNUC_CONST;
EasSyncFolderMsg* eas_sync_folder_msg_new (const gchar* syncKey, gint accountId);
xmlDoc* eas_sync_folder_msg_build_message (EasSyncFolderMsg* self);
void eas_sync_folder_msg_parse_reponse (EasSyncFolderMsg* self, xmlDoc *doc);
GSList* eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self);
GSList* eas_sync_folder_msg_get_updated_folders (EasSyncFolderMsg* self);
GSList* eas_sync_folder_msg_get_deleted_folders (EasSyncFolderMsg* self);
gchar* eas_sync_folder_msg_get_syncKey(EasSyncFolderMsg* self);

G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_MSG_H_ */
