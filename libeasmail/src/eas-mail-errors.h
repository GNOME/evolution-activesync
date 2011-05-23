/*
 *  Filename: eas-mail-errors.h
 */

#ifndef EAS_MAIL_ERRORS_H
#define EAS_MAIL_ERRORS_H

#include <glib.h>

G_BEGIN_DECLS

GQuark eas_mail_error_quark (void);
#define EAS_MAIL_ERROR (eas_mail_error_quark ())

enum {
	EAS_MAIL_ERROR_NONE,
	EAS_MAIL_ERROR_NOTENOUGHMEMORY,
	EAS_MAIL_ERROR_UNKNOWN
};

struct EasMailErrorMap {
	const gchar *error_id;
	gint error_code;
};

gint eas_mail_get_error_code (const gchar *str);

G_END_DECLS

#endif	// EAS_MAIL_ERRORS_H
