
#include "eas-request-base.h"

struct _EasRequestBasePrivate
{
    guint64 requestId;
    gint requestType;
    guint64 accountId;
    struct _EasConnection* connection;
    SoupMessage *soup_message;
    gpointer result;
    EFlag *flag;
	DBusGMethodInvocation *context;
};

#define EAS_REQUEST_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_REQUEST_BASE, EasRequestBasePrivate))



G_DEFINE_TYPE (EasRequestBase, eas_request_base, G_TYPE_OBJECT);

static void
eas_request_base_init (EasRequestBase *object)
{
    EasRequestBasePrivate *priv;

    object->priv = priv = EAS_REQUEST_BASE_PRIVATE (object);

    g_debug ("eas_request_base_init++");
    priv->connection = NULL;
    g_debug ("eas_request_base_init--");
}

static void
eas_request_base_dispose (GObject *object)
{
	EasRequestBase *req = (EasRequestBase *)object;
	EasRequestBasePrivate *priv = req->priv;

	g_debug ("eas_request_base_dispose++");
	if(priv->connection)
	{
		g_debug("not unrefing connection");
		//g_object_unref(priv->connection);
	}
    G_OBJECT_CLASS (eas_request_base_parent_class)->dispose (object);
	g_debug ("eas_request_base_dispose--");
}

static void
eas_request_base_finalize (GObject *object)
{
	g_debug ("eas_request_base_finalize++");
    G_OBJECT_CLASS (eas_request_base_parent_class)->finalize (object);
	g_debug ("eas_request_base_finalize--");
}

static void
eas_request_base_class_init (EasRequestBaseClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);
    void *tmp = parent_class;
    tmp = object_class;

    g_debug ("eas_request_base_class_init++");
    g_type_class_add_private (klass, sizeof (EasRequestBasePrivate));

    object_class->finalize = eas_request_base_finalize;
    object_class->dispose = eas_request_base_dispose;

    g_debug ("eas_request_base_class_init--");
}

/**
 * Should never be called. Exists to define the minium API required for derived
 * classes.
 */
void
eas_request_base_Activate (EasRequestBase *self)
{
    g_error ("Base class message should never be called");
}

/**
 * Should never be called. Exists to define the minium API required for derived
 * classes.
 */
void
eas_request_base_MessageComplete (EasRequestBase *self, xmlDoc* doc)
{
    g_error ("Base class message should never be called");
}

EasRequestType
eas_request_base_GetRequestType (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;

    return priv->requestType;
}

void
eas_request_base_SetRequestType (EasRequestBase* self, EasRequestType type)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetRequestType++");
    priv->requestType = type;
    g_debug ("eas_request_base_SetRequestType--");
}



struct _EasConnection*
eas_request_base_GetConnection (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_GetConnection++ %lx", (unsigned long)priv->connection );
    return priv->connection;
}

void
eas_request_base_SetConnection (EasRequestBase* self, struct _EasConnection* connection)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetConnection++");
    priv->connection = connection;
    g_debug ("eas_request_base_SetConnection--");
}

SoupMessage *
eas_request_base_GetSoupMessage(EasRequestBase *self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_SoupMessage++ %lx", (unsigned long)priv->soup_message );
    return priv->soup_message;
}

void
eas_request_base_SetSoupMessage(EasRequestBase *self, SoupMessage *soup_message)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_SetSoupMessage++");
    priv->soup_message = soup_message;
    g_debug("eas_request_base_SetSoupMessage--");
}


EFlag *
eas_request_base_GetFlag (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_GetFlag+-");
    return priv->flag;
}

void
eas_request_base_SetFlag (EasRequestBase* self, EFlag* flag)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetFlag++");
    priv->flag = flag;
    g_debug ("eas_request_base_SetFlag--");
}

DBusGMethodInvocation*
eas_request_base_GetContext (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_GetContext+-");
    return priv->context;
}

void
eas_request_base_SetContext (EasRequestBase* self, DBusGMethodInvocation* context)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetContext++");
    priv->context = context;
    g_debug ("eas_request_base_SetContext--");
}
