/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include <glib.h>
#include <dbus/dbus-glib.h>

#include "libeastest.h"
#include "../../eas-daemon/src/activesyncd-common-defs.h"
#include "../../libeasclient/eas-logger.h"

#define EAS_TEST_HANDLER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_TEST_HANDLER, EasTestHandlerPrivate))

G_DEFINE_TYPE (EasTestHandler, eas_test_handler, G_TYPE_OBJECT);

struct _EasTestHandlerPrivate
{
    DBusGConnection* bus;
    DBusGProxy *remoteEas;
    GMainLoop* main_loop;
};

static void
eas_test_handler_init (EasTestHandler *object)
{
    EasTestHandlerPrivate *priv = NULL;
    object->priv = priv = EAS_TEST_HANDLER_PRIVATE (object);

	priv->bus = NULL;
	priv->remoteEas = NULL;
	priv->main_loop = NULL;
}

static void
eas_test_handler_dispose (GObject *object)
{
	EasTestHandler *self = (EasTestHandler *)object;
	EasTestHandlerPrivate *priv = self->priv;
	
	g_debug("eas_test_handler_dispose++");
	
	if (priv->main_loop)
	{
		g_main_loop_quit (priv->main_loop);
		g_main_loop_unref (priv->main_loop);
		priv->main_loop = NULL;
	}
	if (priv->bus)
	{
		dbus_g_connection_unref (priv->bus);
		priv->bus = NULL;
	}
	
	G_OBJECT_CLASS (eas_test_handler_parent_class)->dispose (object);
	g_debug("eas_test_handler_dispose--");
}

static void
eas_test_handler_finalize (GObject *object)
{
	g_debug("eas_test_handler_finalize++");
	G_OBJECT_CLASS (eas_test_handler_parent_class)->finalize (object);
	g_debug("eas_test_handler_finalize--");
}

static void
eas_test_handler_class_init (EasTestHandlerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (EasTestHandlerPrivate));

	object_class->finalize = eas_test_handler_finalize;
	object_class->dispose = eas_test_handler_dispose;
}


EasTestHandler*
eas_test_handler_new (void)
{
    GError* error = NULL;
	EasTestHandler *self = NULL;

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif
    dbus_g_thread_init();

	g_debug ("eas_test_handler_new++");

	self = g_object_new (EAS_TYPE_TEST_HANDLER, NULL);
	if (!self) return NULL;

	self->priv->main_loop = g_main_loop_new (NULL, TRUE);
	if (!self->priv->main_loop)
	{
		g_object_unref (self);
		return NULL;
	}

    g_debug ("Connecting to Session D-Bus.");
	self->priv->bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!self->priv->bus)
	{
		g_object_unref (self); // This should clean up all member data
		return NULL;
	}

	g_debug ("Creating a GLib proxy object for Eas.");
    self->priv->remoteEas =  dbus_g_proxy_new_for_name (self->priv->bus,
                                                        EAS_SERVICE_NAME,
                                                        EAS_SERVICE_TEST_OBJECT_PATH,
                                                        EAS_SERVICE_TEST_INTERFACE);
	if (!self->priv->remoteEas)
	{
		g_object_unref (self); // This should clean up all member data
		return NULL;
	}

	dbus_g_proxy_set_default_timeout (self->priv->remoteEas, 1000000);

	g_debug ("eas_test_handler_new--");
	return self;
}

void
eas_test_handler_add_mock_responses (EasTestHandler* self, 
                                     const gchar** mock_responses_array,
                                     GArray *status_codes)
{
	GError *error = NULL;
	gboolean success = FALSE;
	DBusGProxy *proxy = self->priv->remoteEas;

	g_debug("eas_test_handler_add_mock_responses++");
	
    // call DBus API
	g_debug("Call DBus proxy");
    success = dbus_g_proxy_call (proxy, "add_mock_responses",
                                 &error,
                                 G_TYPE_STRV, mock_responses_array,
                                 DBUS_TYPE_G_UINT_ARRAY, status_codes,
                                 G_TYPE_INVALID);

	g_debug("Check DBus Success");
	if (!success)
	{
        if (error)
        {
            g_warning ("[%s][%d][%s]",
                       g_quark_to_string (error->domain),
                       error->code,
                       error->message);
        }
        g_warning ("DBus dbus_g_proxy_call failed");
	}
	g_debug("eas_test_handler_add_mock_responses--");
}
