
#include "eas-request-base.h"

struct _EasRequestBasePrivate
{
	guint64 requestId;
	gint requestType;
	guint64 accountId;
//	Connection* connection;
	gpointer result;
};

#define EAS_REQUEST_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_REQUEST_BASE, EasRequestBasePrivate))



G_DEFINE_TYPE (EasRequestBase, eas_request_base, G_TYPE_OBJECT);

static void
eas_request_base_init (EasRequestBase *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_request_base_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_request_base_parent_class)->finalize (object);
}

static void
eas_request_base_class_init (EasRequestBaseClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasRequestBasePrivate));

	object_class->finalize = eas_request_base_finalize;
}

/**
 * Should never be called. Exists to define the minium API required for derived 
 * classes.
 */
void 
eas_request_base_Activate (EasRequestBase *self)
{
	g_assert(0);
}

/**
 * Should never be called. Exists to define the minium API required for derived 
 * classes.
 */
void
eas_request_base_MessageComplete (EasRequestBase *self, xmlDoc* doc)
{
	g_assert(0);
}

EasRequestType 
eas_request_base_GetRequestType(EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;

    return priv->requestType;
}
