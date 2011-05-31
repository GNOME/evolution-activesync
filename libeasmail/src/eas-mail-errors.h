/*
 *  Filename: eas-mail-errors.h
 */

#ifndef EAS_MAIL_ERRORS_H
#define EAS_MAIL_ERRORS_H

#include <glib.h>

G_BEGIN_DECLS

GQuark
eas_mail_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0)) {
		const gchar *string = "eas-mail-error-quark";
		quark = g_quark_from_static_string (string);
	}

	return quark;
}

#define EAS_MAIL_ERROR (eas_mail_error_quark ())

enum {
	EAS_MAIL_ERROR_NONE,
	EAS_MAIL_ERROR_NOTENOUGHMEMORY,
	EAS_MAIL_ERROR_UNKNOWN
};

G_END_DECLS

#endif	// EAS_MAIL_ERRORS_H
