/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include "eas-msg-base.h"

struct _EasMsgBasePrivate
{
    EasMsgType messageType;
    GObject* accountsObject;
};

#define EAS_MSG_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MSG_BASE, EasMsgBasePrivate))


G_DEFINE_TYPE (EasMsgBase, eas_msg_base, G_TYPE_OBJECT);

static void
eas_msg_base_init (EasMsgBase *object)
{
    EasMsgBasePrivate *priv;
    g_debug ("eas_msg_base_init++");
    object->priv = priv = EAS_MSG_BASE_PRIVATE (object);

	priv->messageType = EAS_MSG_BASE;
	priv->accountsObject = NULL;

    g_debug ("eas_msg_base_init--");
}

static void
eas_msg_base_finalize (GObject *object)
{
    g_debug ("eas_msg_base_finalize++");
    G_OBJECT_CLASS (eas_msg_base_parent_class)->finalize (object);
    g_debug ("eas_msg_base_finalize--");
}

static void
eas_msg_base_class_init (EasMsgBaseClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_debug ("eas_msg_base_class_init++");

    g_type_class_add_private (klass, sizeof (EasMsgBasePrivate));

    object_class->finalize = eas_msg_base_finalize;

    g_debug ("eas_msg_base_class_init--");
}
