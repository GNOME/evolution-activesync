/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#include <glib.h>
#include <gio/gio.h>

#include "libeastest.h"
#include "../../eas-daemon/src/activesyncd-common-defs.h"
#include "../../eas-daemon/src/eas-gdbus-test.h"
#include "../../libeasclient/eas-logger.h"

struct _EasTestHandlerPrivate
{
	EasGDBusTest *proxy;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasTestHandler, eas_test_handler, G_TYPE_OBJECT)

static void
eas_test_handler_init (EasTestHandler *object)
{
	object->priv = eas_test_handler_get_instance_private (object);
	object->priv->proxy = NULL;
}

static void
eas_test_handler_dispose (GObject *object)
{
	EasTestHandler *self = (EasTestHandler *) object;

	g_debug ("eas_test_handler_dispose++");
	g_clear_object (&self->priv->proxy);
	G_OBJECT_CLASS (eas_test_handler_parent_class)->dispose (object);
	g_debug ("eas_test_handler_dispose--");
}

static void
eas_test_handler_finalize (GObject *object)
{
	g_debug ("eas_test_handler_finalize++");
	G_OBJECT_CLASS (eas_test_handler_parent_class)->finalize (object);
	g_debug ("eas_test_handler_finalize--");
}

static void
eas_test_handler_class_init (EasTestHandlerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_test_handler_finalize;
	object_class->dispose = eas_test_handler_dispose;
}

EasTestHandler *
eas_test_handler_new (void)
{
	GError *error = NULL;
	EasTestHandler *self = NULL;
	EasGDBusTest *proxy = NULL;

	g_debug ("eas_test_handler_new++");

	proxy = eas_gdbus_test_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
						       G_DBUS_PROXY_FLAGS_NONE,
						       EAS_SERVICE_NAME,
						       EAS_SERVICE_TEST_OBJECT_PATH,
						       NULL, &error);
	if (!proxy) {
		g_warning ("Failed to create EasTest proxy: %s",
			   error ? error->message : "unknown error");
		g_clear_error (&error);
		return NULL;
	}

	g_dbus_proxy_set_default_timeout (G_DBUS_PROXY (proxy), 1000000);

	self = g_object_new (EAS_TYPE_TEST_HANDLER, NULL);
	self->priv->proxy = proxy;

	g_debug ("eas_test_handler_new--");
	return self;
}

void
eas_test_handler_add_mock_responses (EasTestHandler *self,
				     const gchar **mock_responses_array,
				     GArray *status_codes)
{
	GError *error = NULL;
	GVariantBuilder builder;
	guint i;

	g_debug ("eas_test_handler_add_mock_responses++");

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("au"));
	if (status_codes) {
		for (i = 0; i < status_codes->len; i++)
			g_variant_builder_add (&builder, "u", g_array_index (status_codes, guint, i));
	}

	eas_gdbus_test_call_add_mock_responses_sync (self->priv->proxy,
						     mock_responses_array,
						     g_variant_builder_end (&builder),
						     G_DBUS_CALL_FLAGS_NONE,
						     -1,
						     NULL, &error);
	if (error) {
		g_warning ("add_mock_responses failed: %s", error->message);
		g_error_free (error);
	}

	g_debug ("eas_test_handler_add_mock_responses--");
}
