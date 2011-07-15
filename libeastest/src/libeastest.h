/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _LIBEASTEST_H_
#define _LIBEASTEST_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_TEST_HANDLER             (eas_test_handler_get_type ())
#define EAS_TEST_HANDLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_TEST_HANDLER, EasTestHandler))
#define EAS_TEST_HANDLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_TEST_HANDLER, EasTestHandlerClass))
#define EAS_IS_TEST_HANDLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_TEST_HANDLER))
#define EAS_IS_TEST_HANDLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_TEST_HANDLER))
#define EAS_TEST_HANDLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_TEST_HANDLER, EasTestHandlerClass))

typedef struct _EasTestHandlerClass EasTestHandlerClass;
typedef struct _EasTestHandler EasTestHandler;
typedef struct _EasTestHandlerPrivate EasTestHandlerPrivate;


struct _EasTestHandlerClass
{
	GObjectClass parent_class;
};

struct _EasTestHandler
{
	GObject parent_instance;
	EasTestHandlerPrivate* priv;
};

GType eas_test_handler_get_type (void) G_GNUC_CONST;
EasTestHandler* eas_test_handler_new (void);
void eas_test_handler_add_mock_responses (EasTestHandler* self, const gchar** mock_responses_array, GArray *status_codes);

G_END_DECLS

#endif /* _LIBEASTEST_H_ */
