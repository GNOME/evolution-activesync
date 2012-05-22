/*
 * camel-eas-settings.c
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

#include "camel-eas-settings.h"

#define CAMEL_EAS_SETTINGS_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), CAMEL_TYPE_EAS_SETTINGS, CamelEasSettingsPrivate))

struct _CamelEasSettingsPrivate {
	gboolean check_all;	
	gboolean filter_junk;
	gboolean filter_junk_inbox;
	gchar *account_uid;
};

enum {
	PROP_0,
	PROP_CHECK_ALL,	
	PROP_FILTER_JUNK,
	PROP_FILTER_JUNK_INBOX,
	PROP_ACCOUNT_UID,
	PROP_HOST,
	PROP_PORT,
	PROP_USER,
	PROP_SECURITY_METHOD,
	PROP_AUTH_MECHANISM,
};

G_DEFINE_TYPE_WITH_CODE (
	CamelEasSettings,
	camel_eas_settings,
	CAMEL_TYPE_OFFLINE_SETTINGS,
	G_IMPLEMENT_INTERFACE (
		CAMEL_TYPE_NETWORK_SETTINGS, NULL))

static void
eas_settings_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_CHECK_ALL:
			camel_eas_settings_set_check_all (
				CAMEL_EAS_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_FILTER_JUNK:
			camel_eas_settings_set_filter_junk (
				CAMEL_EAS_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_FILTER_JUNK_INBOX:
			camel_eas_settings_set_filter_junk_inbox (
				CAMEL_EAS_SETTINGS (object),
				g_value_get_boolean (value));
			return;

		case PROP_ACCOUNT_UID:
			camel_eas_settings_set_account_uid (
				CAMEL_EAS_SETTINGS (object),
				g_value_get_string (value));
			return;

		case PROP_AUTH_MECHANISM:
			camel_network_settings_set_auth_mechanism (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;

		case PROP_HOST:
			camel_network_settings_set_host (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;

		case PROP_PORT:
			camel_network_settings_set_port (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_uint (value));
			return;

		case PROP_SECURITY_METHOD:
			camel_network_settings_set_security_method (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_enum (value));
			return;

		case PROP_USER:
			camel_network_settings_set_user (
				CAMEL_NETWORK_SETTINGS (object),
				g_value_get_string (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
eas_settings_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_CHECK_ALL:
			g_value_set_boolean (
				value,
				camel_eas_settings_get_check_all (
				CAMEL_EAS_SETTINGS (object)));
			return;
		
		case PROP_FILTER_JUNK:
			g_value_set_boolean (
				value,
				camel_eas_settings_get_filter_junk (
				CAMEL_EAS_SETTINGS (object)));
			return;

		case PROP_FILTER_JUNK_INBOX:
			g_value_set_boolean (
				value,
				camel_eas_settings_get_filter_junk_inbox (
				CAMEL_EAS_SETTINGS (object)));
			return;

		case PROP_ACCOUNT_UID:
			g_value_set_string (
				value,
				camel_eas_settings_get_account_uid (
				CAMEL_EAS_SETTINGS (object)));
			return;

		case PROP_AUTH_MECHANISM:
			g_value_take_string (
				value,
				camel_network_settings_dup_auth_mechanism (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_HOST:
			g_value_take_string (
				value,
				camel_network_settings_dup_host (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_PORT:
			g_value_set_uint (
				value,
				camel_network_settings_get_port (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_SECURITY_METHOD:
			g_value_set_enum (
				value,
				camel_network_settings_get_security_method (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

		case PROP_USER:
			g_value_take_string (
				value,
				camel_network_settings_dup_user (
				CAMEL_NETWORK_SETTINGS (object)));
			return;

	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
eas_settings_finalize (GObject *object)
{
	CamelEasSettingsPrivate *priv;

	priv = CAMEL_EAS_SETTINGS_GET_PRIVATE (object);

	g_free (priv->account_uid);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (camel_eas_settings_parent_class)->finalize (object);
}

static void
camel_eas_settings_class_init (CamelEasSettingsClass *class)
{
	GObjectClass *object_class;

	g_type_class_add_private (class, sizeof (CamelEasSettingsPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = eas_settings_set_property;
	object_class->get_property = eas_settings_get_property;
	object_class->finalize = eas_settings_finalize;

	g_object_class_install_property (
		object_class,
		PROP_CHECK_ALL,
		g_param_spec_boolean (
			"check-all",
			"Check All",
			"Check all folders for new messages",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_FILTER_JUNK,
		g_param_spec_boolean (
			"filter-junk",
			"Filter Junk",
			"Whether to filter junk from all folders",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_FILTER_JUNK_INBOX,
		g_param_spec_boolean (
			"filter-junk-inbox",
			"Filter Junk Inbox",
			"Whether to filter junk from Inbox only",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_ACCOUNT_UID,
		g_param_spec_string (
			"account-uid",
			"Account UID",
			"Account UID",
			NULL,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	/* Inherited from CamelNetworkSettings. */
	g_object_class_override_property (
		object_class,
		PROP_AUTH_MECHANISM,
		"auth-mechanism");

	g_object_class_override_property (
		object_class,
		PROP_HOST,
		"host");

	g_object_class_override_property (
		object_class,
		PROP_PORT,
		"port");

	g_object_class_override_property (
		object_class,
		PROP_SECURITY_METHOD,
		"security-method");

	g_object_class_override_property (
		object_class,
		PROP_USER,
		"user");

}

static void
camel_eas_settings_init (CamelEasSettings *settings)
{
	settings->priv = CAMEL_EAS_SETTINGS_GET_PRIVATE (settings);
}


const gchar *
camel_eas_settings_get_account_uid (CamelEasSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_EAS_SETTINGS (settings), NULL);

	return settings->priv->account_uid;
}

void
camel_eas_settings_set_account_uid (CamelEasSettings *settings,
                               	    const gchar *account_uid)
{
	g_return_if_fail (CAMEL_IS_EAS_SETTINGS (settings));

	g_free (settings->priv->account_uid);
	settings->priv->account_uid= g_strdup (account_uid);

	g_object_notify (G_OBJECT (settings), "account-uid");
}

/**
 * camel_eas_settings_get_check_all:
 * @settings: a #CamelEasSettings
 *
 * Returns whether to check all folders for new messages.
 *
 * Returns: whether to check all folders for new messages
 *
 * Since: 3.4
 **/
gboolean
camel_eas_settings_get_check_all (CamelEasSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_EAS_SETTINGS (settings), FALSE);

	return settings->priv->check_all;
}

/**
 * camel_eas_settings_set_check_all:
 * @settings: a #CamelEasSettings
 * @check_all: whether to check all folders for new messages
 *
 * Sets whether to check all folders for new messages.
 *
 * Since: 3.4
 **/
void
camel_eas_settings_set_check_all (CamelEasSettings *settings,
                                  gboolean check_all)
{
	g_return_if_fail (CAMEL_IS_EAS_SETTINGS (settings));

	settings->priv->check_all = check_all;

	g_object_notify (G_OBJECT (settings), "check-all");
}

gboolean
camel_eas_settings_get_filter_junk (CamelEasSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_EAS_SETTINGS (settings), FALSE);

	return settings->priv->filter_junk;
}

void
camel_eas_settings_set_filter_junk (CamelEasSettings *settings,
                                    gboolean filter_junk)
{
	g_return_if_fail (CAMEL_IS_EAS_SETTINGS (settings));

	settings->priv->filter_junk = filter_junk;

	g_object_notify (G_OBJECT (settings), "filter-junk");
}

gboolean
camel_eas_settings_get_filter_junk_inbox (CamelEasSettings *settings)
{
	g_return_val_if_fail (CAMEL_IS_EAS_SETTINGS (settings), FALSE);

	return settings->priv->filter_junk_inbox;
}

void
camel_eas_settings_set_filter_junk_inbox (CamelEasSettings *settings,
                                          gboolean filter_junk_inbox)
{
	g_return_if_fail (CAMEL_IS_EAS_SETTINGS (settings));

	settings->priv->filter_junk_inbox = filter_junk_inbox;

	g_object_notify (G_OBJECT (settings), "filter-junk-inbox");
}
