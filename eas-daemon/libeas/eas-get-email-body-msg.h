/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_GET_EMAIL_BODY_MSG_H_
#define _EAS_GET_EMAIL_BODY_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_BODY_MSG             (eas_get_email_body_msg_get_type ())
#define EAS_GET_EMAIL_BODY_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsg))
#define EAS_GET_EMAIL_BODY_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))
#define EAS_IS_GET_EMAIL_BODY_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_IS_GET_EMAIL_BODY_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_GET_EMAIL_BODY_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))

typedef struct _EasGetEmailBodyMsgClass EasGetEmailBodyMsgClass;
typedef struct _EasGetEmailBodyMsg EasGetEmailBodyMsg;
typedef struct _EasGetEmailBodyMsgPrivate EasGetEmailBodyMsgPrivate;

struct _EasGetEmailBodyMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailBodyMsg
{
	EasMsgBase parent_instance;

	EasGetEmailBodyMsgPrivate* priv;
};

GType eas_get_email_body_msg_get_type (void) G_GNUC_CONST;
EasGetEmailBodyMsg* eas_get_email_body_msg_new (const gchar* serverUid);
xmlDoc* eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self);
void eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, xmlDoc* doc);

gchar* eas_get_email_body_msg_get_syncKey(EasGetEmailBodyMsg* self);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_BODY_MSG_H_ */
