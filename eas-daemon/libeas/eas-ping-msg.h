/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_SYNC_MSG_H_
#define _EAS_PING_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PING_MSG             (eas_ping_msg_get_type ())
#define EAS_PING_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PING_MSG, EasPingMsg))
#define EAS_PING_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PING_MSG, EasPingMsgClass))
#define EAS_IS_PING_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PING_MSG))
#define EAS_IS_PING_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PING_MSG))
#define EAS_PING_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PING_MSG, EasPingMsgClass))

typedef struct _EasPingMsgClass EasPingMsgClass;
typedef struct _EasPingMsg EasPingMsg;
typedef struct _EasPingMsgPrivate EasPingMsgPrivate;

struct _EasPingMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasPingMsg
{
	EasMsgBase parent_instance;
	
	EasPingMsgPrivate *priv;
};

GType eas_ping_msg_get_type (void) G_GNUC_CONST;

EasPingMsg* eas_ping_msg_new ();
xmlDoc* eas_ping_msg_build_message (EasPingMsg* self, const gchar* accountId, const gchar *heartbeat, GSList *folders);
gboolean eas_ping_msg_parse_response (EasPingMsg* self, xmlDoc *doc, GError** error);


G_END_DECLS

#endif /* _EAS_PING_MSG_H_ */
