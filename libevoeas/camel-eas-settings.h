/*
 * camel-eas-settings.h
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef CAMEL_EAS_SETTINGS_H
#define CAMEL_EAS_SETTINGS_H

#include <camel/camel.h>

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_SETTINGS \
	(camel_eas_settings_get_type ())
#define CAMEL_EAS_SETTINGS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_SETTINGS, CamelEasSettings))
#define CAMEL_EAS_SETTINGS_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_SETTINGS, CamelEasSettingsClass))
#define CAMEL_IS_EAS_SETTINGS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_SETTINGS))
#define CAMEL_IS_EAS_SETTINGS_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_SETTINGS))
#define CAMEL_EAS_SETTINGS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_SETTINGS))

G_BEGIN_DECLS

typedef struct _CamelEasSettings CamelEasSettings;
typedef struct _CamelEasSettingsClass CamelEasSettingsClass;
typedef struct _CamelEasSettingsPrivate CamelEasSettingsPrivate;

struct _CamelEasSettings {
	CamelOfflineSettings parent;
	CamelEasSettingsPrivate *priv;
};

struct _CamelEasSettingsClass {
	CamelOfflineSettingsClass parent_class;
};

GType		camel_eas_settings_get_type			(void) G_GNUC_CONST;
gboolean	camel_eas_settings_get_check_all		(CamelEasSettings *settings);
void		camel_eas_settings_set_check_all		(CamelEasSettings *settings,
								 gboolean check_all);
gboolean	camel_eas_settings_get_filter_junk		(CamelEasSettings *settings);
void		camel_eas_settings_set_filter_junk		(CamelEasSettings *settings,
								 gboolean filter_junk);
gboolean	camel_eas_settings_get_filter_junk_inbox	(CamelEasSettings *settings);
void		camel_eas_settings_set_filter_junk_inbox	(CamelEasSettings *settings,
								 gboolean filter_junk_inbox);
const gchar *	camel_eas_settings_get_account_uid		(CamelEasSettings *settings);
void		camel_eas_settings_set_account_uid		(CamelEasSettings *settings,
								 const gchar *uid);

G_END_DECLS

#endif /* CAMEL_EAS_SETTINGS_H */
