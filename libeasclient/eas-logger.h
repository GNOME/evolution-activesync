#ifndef _EAS_LOGGER_H_
#define _EAS_LOGGER_H_

#include <glib.h>

void eas_logger (const gchar *log_domain,
		 GLogLevelFlags log_level,
		 const gchar *message,
		 gpointer user_data);

#endif
