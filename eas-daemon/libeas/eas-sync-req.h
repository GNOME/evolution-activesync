/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_SYNC_REQ_H_
#define _EAS_SYNC_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_REQ             (eas_sync_req_get_type ())
#define EAS_SYNC_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_REQ, EasSyncReq))
#define EAS_SYNC_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_REQ, EasSyncReqClass))
#define EAS_IS_SYNC_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_REQ))
#define EAS_IS_SYNC_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_REQ))
#define EAS_SYNC_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_REQ, EasSyncReqClass))

typedef struct _EasSyncReqClass EasSyncReqClass;
typedef struct _EasSyncReq EasSyncReq;
typedef struct _EasSyncReqPrivate EasSyncReqPrivate;

struct _EasSyncReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSyncReq
{
	EasRequestBase parent_instance;
	
	EasSyncReqPrivate *priv;
};

GType eas_sync_req_get_type (void) G_GNUC_CONST;

gboolean eas_sync_req_Activate (EasSyncReq *self, const gchar* syncKey, guint64 accountID, EFlag *flag, const gchar* folderId, EasItemType type, GError** error);
void eas_sync_req_MessageComplete (EasSyncReq *self, xmlDoc* doc, GError* error);
gboolean eas_sync_req_ActivateFinish (EasSyncReq* self, gchar** ret_sync_key, gboolean *ret_more_available, GSList** added_items, GSList** updated_items, GSList** deleted_items, GError** error);

G_END_DECLS

#endif /* _EAS_SYNC_REQ_H_ */
