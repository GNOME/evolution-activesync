/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_PROVISION_MSG_H_
#define _EAS_PROVISION_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_MSG             (eas_provision_msg_get_type ())
#define EAS_PROVISION_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsg))
#define EAS_PROVISION_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))
#define EAS_IS_PROVISION_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_MSG))
#define EAS_IS_PROVISION_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_MSG))
#define EAS_PROVISION_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))

typedef struct _EasProvisionMsgClass EasProvisionMsgClass;
typedef struct _EasProvisionMsg EasProvisionMsg;
typedef struct _EasProvisionMsgPrivate EasProvisionMsgPrivate;

struct _EasProvisionMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasProvisionMsg
{
	EasMsgBase parent_instance;

	EasProvisionMsgPrivate *priv;
};

GType eas_provision_msg_get_type (void) G_GNUC_CONST;
xmlDoc* eas_provision_msg_build_message (EasProvisionMsg* self);
void eas_provision_msg_parse_response (EasProvisionMsg* self, xmlDoc* doc, GError** error);
EasProvisionMsg* eas_provision_msg_new (void);
gchar* eas_provision_msg_get_policy_key (EasProvisionMsg* self);
gchar* eas_provision_msg_get_policy_status (EasProvisionMsg* self);
void eas_provision_msg_set_policy_key (EasProvisionMsg* self, gchar* policyKey);
void eas_provision_msg_set_policy_status (EasProvisionMsg* self, gchar* policyStatus);

G_END_DECLS

#endif /* _EAS_PROVISION_MSG_H_ */
