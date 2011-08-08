/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */

#ifndef _EAS_TEST_H_
#define _EAS_TEST_H_

#include <glib-object.h>
#include <dbus/dbus-glib.h>


G_BEGIN_DECLS

#define EAS_TYPE_TEST             (eas_test_get_type ())
#define EAS_TEST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_TEST, EasTest))
#define EAS_TEST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_TEST, EasTestClass))
#define EAS_IS_TEST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_TEST))
#define EAS_IS_TEST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_TEST))
#define EAS_TEST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_TEST, EasTestClass))

typedef struct _EasTestClass EasTestClass;
typedef struct _EasTest EasTest;


struct _EasTestClass {
	GObjectClass parent_class;
};

struct _EasTest {
	GObject parent_instance;
};

GType eas_test_get_type (void) G_GNUC_CONST;
EasTest* eas_test_new (void);
void eas_test_add_mock_responses (EasTest* self, const gchar** mock_responses_array, const GArray *status_codes, DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_TEST_H_ */
