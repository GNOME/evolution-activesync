/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */

#include "eas-test.h"
#include "eas-gdbus-test.h"
#include "eas-connection.h"

G_DEFINE_TYPE (EasTest, eas_test, G_TYPE_OBJECT);

#define EAS_TEST_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_TEST, EasTestPrivate))

struct _EasTestPrivate {
	EasGDBusTestSkeleton *skeleton;
};

static gboolean
on_handle_add_mock_responses (EasGDBusTest *obj G_GNUC_UNUSED, GDBusMethodInvocation *invocation,
			      const gchar *const *mock_responses_array,
			      GVariant *mock_status_codes_variant, EasTest *self)
{
	GArray *status_codes = NULL;
	gsize n = g_variant_n_children (mock_status_codes_variant);

	if (n > 0) {
		guint i;
		status_codes = g_array_sized_new (FALSE, FALSE, sizeof (guint), n);
		for (i = 0; i < n; i++) {
			guint val;
			g_variant_get_child (mock_status_codes_variant, i, "u", &val);
			g_array_append_val (status_codes, val);
		}
	}

	eas_test_add_mock_responses (self, mock_responses_array, status_codes, invocation);

	if (status_codes)
		g_array_free (status_codes, TRUE);

	return TRUE;
}

static void
eas_test_init (EasTest *object)
{
	EasTestPrivate *priv;
	g_debug ("eas_test_init+-");
	object->priv = priv = EAS_TEST_PRIVATE (object);

	priv->skeleton = eas_gdbus_test_skeleton_new ();
	g_signal_connect (priv->skeleton, "handle-add-mock-responses",
			  G_CALLBACK (on_handle_add_mock_responses), object);
}

static void
eas_test_dispose (GObject *object)
{
	EasTest *self = EAS_TEST (object);
	g_clear_object (&self->priv->skeleton);
	G_OBJECT_CLASS (eas_test_parent_class)->dispose (object);
}

static void
eas_test_finalize (GObject *object)
{
	g_debug ("eas_test_finalize++");
	G_OBJECT_CLASS (eas_test_parent_class)->finalize (object);
	g_debug ("eas_test_finalize--");
}

static void
eas_test_class_init (EasTestClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (EasTestPrivate));
	object_class->dispose = eas_test_dispose;
	object_class->finalize = eas_test_finalize;
}

EasTest*
eas_test_new (void)
{
	EasTest *self = NULL;
	g_debug ("eas_test_new++");
	self = g_object_new (EAS_TYPE_TEST, NULL);
	g_debug ("eas_test_new--");
	return self;
}

GDBusInterfaceSkeleton *
eas_test_get_skeleton (EasTest *self)
{
	return G_DBUS_INTERFACE_SKELETON (self->priv->skeleton);
}

void
eas_test_add_mock_responses (EasTest* self,
			     const gchar** mock_responses_array,
			     const GArray *mock_status_codes,
			     GDBusMethodInvocation* context)
{
	g_debug ("eas_test_add_mock_responses++");

	if (!mock_responses_array) {
		GError *error = NULL;
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,
			     "Array of files to be mocked was NULL");
		g_dbus_method_invocation_return_gerror(context, error);
		g_error_free (error);
	} else {
		eas_connection_add_mock_responses (mock_responses_array, mock_status_codes);
		g_dbus_method_invocation_return_value (context, NULL);
	}

	g_debug ("eas_test_add_mock_responses--");
}
