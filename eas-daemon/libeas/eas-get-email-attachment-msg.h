/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_GET_EMAIL_ATTACHMENT_MSG_H_
#define _EAS_GET_EMAIL_ATTACHMENT_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG             (eas_get_email_attachment_msg_get_type ())
#define EAS_GET_EMAIL_ATTACHMENT_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsg))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))

typedef struct _EasGetEmailAttachmentMsgClass EasGetEmailAttachmentMsgClass;
typedef struct _EasGetEmailAttachmentMsg EasGetEmailAttachmentMsg;
typedef struct _EasGetEmailAttachmentMsgPrivate EasGetEmailAttachmentMsgPrivate;

struct _EasGetEmailAttachmentMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailAttachmentMsg
{
	EasMsgBase parent_instance;
	
	EasGetEmailAttachmentMsgPrivate* priv;
};

GType eas_get_email_attachment_msg_get_type (void) G_GNUC_CONST;
EasGetEmailAttachmentMsg* eas_get_email_attachment_msg_new (const gchar *fileReference, const gchar* directoryPath);
xmlDoc* eas_get_email_attachment_msg_build_message (EasGetEmailAttachmentMsg* self);
gboolean eas_get_email_attachment_msg_parse_response (EasGetEmailAttachmentMsg* self, xmlDoc* doc, GError** error);

gchar* eas_get_email_attachment_msg_get_syncKey(EasGetEmailAttachmentMsg* self);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_ATTACHMENT_MSG_H_ */
