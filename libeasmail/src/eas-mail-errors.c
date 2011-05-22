/*
 *  Filename: eas-mail-errors.c
 */

#include "eas-mail-errors.h"

static GHashTable *eas_mail_error_hash = NULL;
static GOnce setup_error_once = G_ONCE_INIT;

static struct EasMailErrorMap
eas_mail_errors [] =
{
	{"ErrorNotEnoughMemory",				EAS_MAIL_ERROR_NOTENOUGHMEMORY},
};

static gpointer
setup_error_map (gpointer data)
{
	gint i;

	eas_mail_error_hash = g_hash_table_new	(g_str_hash, g_str_equal);
	for (i = 0; i < G_N_ELEMENTS(eas_mail_errors); i++)
		g_hash_table_insert	(eas_mail_error_hash, (gpointer) eas_mail_errors[i].error_id,
					 GINT_TO_POINTER (eas_mail_errors[i].error_code));
	return NULL;
}

gint
eas_mail_get_error_code (const gchar *str)
{
	gint error_code = EAS_MAIL_ERROR_UNKNOWN;
	gpointer data;

	g_once (&setup_error_once, setup_error_map, NULL);

	data = g_hash_table_lookup (eas_mail_error_hash, (gconstpointer) str);
	if (data)
		error_code = GPOINTER_TO_INT (data);

	return error_code;
}

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
