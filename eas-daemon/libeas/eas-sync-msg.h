/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_SYNC_MSG_H_
#define _EAS_SYNC_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_MSG             (eas_sync_msg_get_type ())
#define EAS_SYNC_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_MSG, EasSyncMsg))
#define EAS_SYNC_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_MSG, EasSyncMsgClass))
#define EAS_IS_SYNC_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_MSG))
#define EAS_IS_SYNC_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_MSG))
#define EAS_SYNC_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_MSG, EasSyncMsgClass))

typedef struct _EasSyncMsgClass EasSyncMsgClass;
typedef struct _EasSyncMsg EasSyncMsg;
typedef struct _EasSyncMsgPrivate EasSyncMsgPrivate;

struct _EasSyncMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasSyncMsg
{
	EasMsgBase parent_instance;
	
	EasSyncMsgPrivate *priv;
};

GType eas_sync_msg_get_type (void) G_GNUC_CONST;

EasSyncMsg* eas_sync_msg_new (const gchar* syncKey, gint accountId, gchar *FolderID, EasItemType type);
xmlDoc* eas_sync_msg_build_message (EasSyncMsg* self, gboolean getChanges, GSList *added, GSList *updated, GSList *deleted);
void eas_sync_msg_parse_reponse (EasSyncMsg* self, xmlDoc *doc);
GSList* eas_sync_msg_get_added_items (EasSyncMsg* self);
GSList* eas_sync_msg_get_updated_items (EasSyncMsg* self);
GSList* eas_sync_msg_get_deleted_items (EasSyncMsg* self);
gchar* eas_sync_msg_get_syncKey(EasSyncMsg* self);

G_END_DECLS

#endif /* _EAS_SYNC_MSG_H_ */
