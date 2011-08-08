/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */

#include "eas-test.h"
#include "eas-test-stub.h"
#include "eas-connection.h"

G_DEFINE_TYPE (EasTest, eas_test, G_TYPE_OBJECT);

static void
eas_test_init (EasTest *object)
{
	g_debug ("eas_test_init+-");
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

	g_debug ("eas_test_class_init++");

	dbus_g_object_type_install_info (EAS_TYPE_TEST,
					 &dbus_glib_eas_test_object_info);

	object_class->finalize = eas_test_finalize;
	g_debug ("eas_test_class_init--");
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

void
eas_test_add_mock_responses (EasTest* self,
			     const gchar** mock_responses_array,
			     const GArray *mock_status_codes,
			     DBusGMethodInvocation* context)
{
	g_debug ("eas_test_add_mock_responses++");

	if (!mock_responses_array) {
		GError *error = NULL;
		g_set_error (&error,
			     EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,
			     "Array of files to be mocked was NULL");
		dbus_g_method_return_error (context, error);
		g_error_free (error);
	} else {
		eas_connection_add_mock_responses (mock_responses_array, mock_status_codes);
		dbus_g_method_return (context);
	}

	g_debug ("eas_test_add_mock_responses--");
}
