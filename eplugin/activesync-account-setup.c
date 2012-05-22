/*
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
 *
 * Authors:
 *		Sushma Rai <rsushma@novell.com>
 *
 * Copyright (C) 1999-2008 Novell, Inc. (www.novell.com)
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libedataserver/e-account.h>
#include <libedataserver/eds-version.h>

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <e-util/e-dialog-utils.h>
#include "mail/em-account-editor.h"
#include "mail/em-config.h"

#include <eas-account.h>
#include <eas-account-list.h>
#include <libeasmail.h>
#include "eas-account-listener.h"

#if EDS_CHECK_VERSION(3,1,0)
#define MODIFIED_ACCT modified_account
#else
#define MODIFIED_ACCT account
#endif

#if EDS_CHECK_VERSION (3,3,90)
#include "camel-eas-settings.h"
#endif

static EasAccountListener *config_listener = NULL;

GtkWidget *org_gnome_activesync_server_url(EPlugin *epl, EConfigHookItemFactoryData *data);
gboolean org_gnome_activesync_check_options(EPlugin *epl, EConfigHookPageCheckData *data);
#if EDS_CHECK_VERSION (3,3,90)
void org_gnome_activesync_commit (EPlugin *epl, EMConfigTargetSettings *target_account);
#else
void org_gnome_activesync_commit (EPlugin *epl, EMConfigTargetAccount *target_account);
#endif

#define EVOLUTION_ACCOUNT_URL_FORMAT "eas://%s/;account_uid=%s;passwd_exp_warn_period=7;ad_limit=500;check_all;owa_url=%s"

#if !EDS_CHECK_VERSION (3,3,90)
static void
update_url_account (EAccount *account, e_account_item_t item, const gchar *surl)
{
	CamelURL *url;
	gchar *url_string;
	const gchar *target_url;

	if (!account)
		return;

	target_url = e_account_get_string (account, item);
	if (target_url && target_url[0] != '\0')
		url = camel_url_new (target_url, NULL);
	else
		return;

	camel_url_set_param (url, "owa_url", surl);
	camel_url_set_param (url, "account_uid", e_account_get_string (account, E_ACCOUNT_ID_ADDRESS));
	
	url_string = camel_url_to_string (url, 0);
	e_account_set_string (account, item, url_string);
	g_free (url_string);
	camel_url_free (url);


}
#endif

#if EDS_CHECK_VERSION(3,3,90)
static void
url_entry_changed(GtkWidget *entry, EConfig *config)
{
	gchar *uri;
	EMConfigTargetSettings *target = (EMConfigTargetSettings *)config->target;

	uri = g_strdup (gtk_entry_get_text((GtkEntry *)entry));

	g_strstrip (uri);

	if (uri && uri[0]) {
		const char *address = target->email_address;
		GConfClient *client = gconf_client_get_default ();
		char *key;

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
		gconf_client_set_string (client, key, uri, NULL);
		g_free (key);
		g_object_unref (client);

		camel_eas_settings_set_account_uid ((CamelEasSettings *)target->storage_settings, address);
		camel_eas_settings_set_account_uid ((CamelEasSettings *)target->transport_settings, address);

	} 

	g_free (uri);
}
#else
static void
url_entry_changed(GtkWidget *entry, EConfig *config)
{
	gchar *uri;
	EMConfigTargetAccount *target = (EMConfigTargetAccount *)config->target;

	uri = g_strdup (gtk_entry_get_text((GtkEntry *)entry));

	g_strstrip (uri);

	if (uri && uri[0]) {
		const char *address = e_account_get_string (target->MODIFIED_ACCT,
							    E_ACCOUNT_ID_ADDRESS);
		GConfClient *client = gconf_client_get_default ();
		char *key;

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
		gconf_client_set_string (client, key, uri, NULL);
		g_free (key);
		g_object_unref (client);
		update_url_account (target->MODIFIED_ACCT,
				    E_ACCOUNT_SOURCE_URL, uri);
		update_url_account (target->MODIFIED_ACCT,
				    E_ACCOUNT_TRANSPORT_URL, uri);

	} 

	g_free (uri);
}
#endif

#if EDS_CHECK_VERSION(3,3,90)
static void
username_entry_changed (GtkWidget *entry, EConfig *config)
{
	gchar *name;
	EMConfigTargetSettings *target = (EMConfigTargetSettings *)config->target;

	name = g_strdup (gtk_entry_get_text((GtkEntry *)entry));

	g_strstrip (name);

	if (name && name[0]) {
		const char *address = target->email_address;
		GConfClient *client = gconf_client_get_default ();
		char *key;

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);
		gconf_client_set_string (client, key, name, NULL);
		g_free (key);
		g_object_unref (client);
	} 

	g_free (name);

}
#else
static void
username_entry_changed (GtkWidget *entry, EConfig *config)
{
	gchar *name;
	EMConfigTargetAccount *target = (EMConfigTargetAccount *)config->target;

	name = g_strdup (gtk_entry_get_text((GtkEntry *)entry));

	g_strstrip (name);

	if (name && name[0]) {
		const char *address = e_account_get_string (target->MODIFIED_ACCT,
							    E_ACCOUNT_ID_ADDRESS);
		GConfClient *client = gconf_client_get_default ();
		char *key;

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);
		gconf_client_set_string (client, key, name, NULL);
		g_free (key);
		g_object_unref (client);
	} 

	g_free (name);

}
#endif

#if EDS_CHECK_VERSION(3,3,90)
static void
discover_server_url (GtkWidget *button, EConfig  *config)
{
	EMConfigTargetSettings *target = (EMConfigTargetSettings *)config->target;
	EasEmailHandler *handler;
	GError *error = NULL;
	const char *address;
	gchar *uri = NULL;
	char *username, *key;
	GtkWidget *entry = (GtkWidget *) g_object_get_data ((GObject *) button, "url-entry");
	GConfClient *client = gconf_client_get_default();

	address = target->email_address;

	handler = eas_mail_handler_new(address, &error);
	if (error) {
		g_warning ("Unable to create mailHandler. We don't support auto-discover: %s\n", error->message);
		g_error_free (error);
		gtk_widget_set_sensitive (button, FALSE);
		return;
	} 

	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);
	username = gconf_client_get_string (client, key, NULL);
	if (!username || !*username || strcmp (username, address) == 0) {
		g_free (username);
		username = NULL;
	}
	g_object_unref (client);
        eas_mail_handler_autodiscover(
            handler,
            address,
            username,
            &uri,
            NULL,
            &error);
	
	if (!error && uri && *uri)
		gtk_entry_set_text ((GtkEntry *) entry, uri);

	camel_eas_settings_set_account_uid ((CamelEasSettings *)target->storage_settings, address);
	camel_eas_settings_set_account_uid ((CamelEasSettings *)target->transport_settings, address);

	g_free (key);
	g_free (username);
	g_object_unref (handler);
}
#else
static void
discover_server_url (GtkWidget *button, EConfig  *config)
{
	EMConfigTargetAccount *target = (EMConfigTargetAccount *)config->target;
	EasEmailHandler *handler;
	GError *error = NULL;
	const char *address;
	gchar *uri = NULL;
	char *username, *key;
	GtkWidget *entry = (GtkWidget *) g_object_get_data ((GObject *) button, "url-entry");
	GConfClient *client = gconf_client_get_default();

	address = e_account_get_string (target->MODIFIED_ACCT,
					E_ACCOUNT_ID_ADDRESS);

	handler = eas_mail_handler_new(address, &error);
	if (error) {
		g_warning ("Unable to create mailHandler. We don't support auto-discover: %s\n", error->message);
		g_error_free (error);
		gtk_widget_set_sensitive (button, FALSE);
		return;
	} 

	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);
	username = gconf_client_get_string (client, key, NULL);
	if (!username || !*username || strcmp (username, address) == 0) {
		g_free (username);
		username = NULL;
	}
	g_object_unref (client);
        eas_mail_handler_autodiscover(
            handler,
            address,
            username,
            &uri,
            NULL,
            &error);
	
	if (!error && uri && *uri)
		gtk_entry_set_text ((GtkEntry *) entry, uri);

	update_url_account (target->MODIFIED_ACCT,
			    E_ACCOUNT_SOURCE_URL, uri);
	update_url_account (target->MODIFIED_ACCT,
			    E_ACCOUNT_TRANSPORT_URL, uri);

	g_free (key);
	g_free (username);
	g_object_unref (handler);
}
#endif

#if EDS_CHECK_VERSION(3,3,90)
GtkWidget *
org_gnome_activesync_server_url(EPlugin *epl, EConfigHookItemFactoryData *data)
{
	EMConfigTargetSettings *target_account;
	const gchar *source_url;
	gchar *server_url = NULL, *username, *key;
	const char *address;
	GtkWidget *url_entry, *username_entry;
	gint row;
	GtkWidget *hbox, *label, *button;
	GConfClient *client = gconf_client_get_default();
	EasEmailHandler* handler;
	GError *error = NULL;
	CamelSettings *settings;
	CamelEasSettings *eas_settings;
	CamelNetworkSettings *network_settings;

	target_account = (EMConfigTargetSettings *)data->config->target;
	settings = target_account->storage_settings;

	if (!CAMEL_IS_EAS_SETTINGS (settings)) {
		if (data->old
		    && (label = g_object_get_data((GObject *)data->old, "authenticate-label")))
			gtk_widget_destroy(label);

		return NULL;
	}

	
	if (data->old) {
		return data->old;
	}

	eas_settings = CAMEL_EAS_SETTINGS (settings);
	network_settings = CAMEL_NETWORK_SETTINGS (settings);


	address = target_account->email_address;

	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);

	username = gconf_client_get_string (client, key, NULL);

	if (!username || !*username) {
		gconf_client_set_string (client, key, address, NULL);
		username = g_strdup (address);
	}
	g_free (key);

	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
	server_url = gconf_client_get_string (client, key, NULL);
	g_object_unref (client);
	g_free (key);

	g_object_get (data->parent, "n-rows", &row, NULL);

	label = gtk_label_new_with_mnemonic (_("User_name:"));
	gtk_widget_show (label);

	username_entry = gtk_entry_new ();
	gtk_widget_show (username_entry);
	if (username)
		gtk_entry_set_text (GTK_ENTRY (username_entry), username);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), username_entry);

	g_signal_connect (username_entry, "changed", G_CALLBACK (username_entry_changed), data->config);

	gtk_table_attach (GTK_TABLE (data->parent), label, 0, 1, row, row+1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (data->parent), username_entry, 1, 2, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);
	
	row++;

	hbox = gtk_hbox_new (FALSE, 6);
	label = gtk_label_new_with_mnemonic(_("_Server URL:"));
	gtk_widget_show(label);

	url_entry = gtk_entry_new();

	gtk_label_set_mnemonic_widget((GtkLabel *)label, url_entry);

	button = gtk_button_new_with_mnemonic (_("_Auto Detect"));

	gtk_box_pack_start (GTK_BOX (hbox), url_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);

	gtk_table_attach (GTK_TABLE (data->parent), label, 0, 1, row, row+1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (data->parent), hbox, 1, 2, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);

	g_signal_connect (url_entry, "changed", G_CALLBACK(url_entry_changed), data->config);

	if (server_url)
		gtk_entry_set_text(GTK_ENTRY (url_entry), server_url);
	g_object_set_data ((GObject *)button, "url-entry", (gpointer) url_entry);
	g_signal_connect (button, "clicked", G_CALLBACK(discover_server_url), data->config);


	g_free (server_url);
	g_free (username);

	return hbox;
}
#else
/* used by editor and assistant - same code */
GtkWidget *
org_gnome_activesync_server_url(EPlugin *epl, EConfigHookItemFactoryData *data)
{
	EMConfigTargetAccount *target_account;
	const gchar *source_url;
	gchar *server_url = NULL, *username, *key;
	const char *address;
	GtkWidget *url_entry, *username_entry;
	CamelURL *url;
	gint row;
	GtkWidget *hbox, *label, *button;
	GConfClient *client = gconf_client_get_default();
	EasEmailHandler* handler;
	GError *error = NULL;

	target_account = (EMConfigTargetAccount *)data->config->target;
	source_url = e_account_get_string (target_account->MODIFIED_ACCT,
					   E_ACCOUNT_SOURCE_URL);
	if (source_url && source_url[0] != '\0')
		url = camel_url_new(source_url, NULL);
	else
		url = NULL;
	if (url == NULL
	    || strcmp(url->protocol, "eas") != 0) {
		if (url)
			camel_url_free(url);

		if (data->old
		    && (label = g_object_get_data((GObject *)data->old, "authenticate-label")))
			gtk_widget_destroy(label);

		/* TODO: we could remove 'owa-url' from the url,
		   but that will lose it if we come back.  Maybe a commit callback could do it */

		return NULL;
	}

	if (data->old) {
		camel_url_free(url);
		return data->old;
	}


	address = e_account_get_string (target_account->MODIFIED_ACCT,
					E_ACCOUNT_ID_ADDRESS);
	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/username", address);

	username = gconf_client_get_string (client, key, NULL);

	if (!username || !*username) {
		gconf_client_set_string (client, key, address, NULL);
		username = g_strdup (address);
	}
	g_free (key);

	key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
	server_url = gconf_client_get_string (client, key, NULL);
	g_object_unref (client);
	g_free (key);

	g_object_get (data->parent, "n-rows", &row, NULL);

	label = gtk_label_new_with_mnemonic (_("User_name:"));
	gtk_widget_show (label);

	username_entry = gtk_entry_new ();
	gtk_widget_show (username_entry);
	if (username)
		gtk_entry_set_text (GTK_ENTRY (username_entry), username);

	gtk_label_set_mnemonic_widget (GTK_LABEL (label), username_entry);

	g_signal_connect (username_entry, "changed", G_CALLBACK (username_entry_changed), data->config);

	gtk_table_attach (GTK_TABLE (data->parent), label, 0, 1, row, row+1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (data->parent), username_entry, 1, 2, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);
	
	row++;

	hbox = gtk_hbox_new (FALSE, 6);
	label = gtk_label_new_with_mnemonic(_("_Server URL:"));
	gtk_widget_show(label);

	url_entry = gtk_entry_new();

	camel_url_free (url);
	gtk_label_set_mnemonic_widget((GtkLabel *)label, url_entry);

	button = gtk_button_new_with_mnemonic (_("_Auto Detect"));

	gtk_box_pack_start (GTK_BOX (hbox), url_entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show_all(hbox);

	gtk_table_attach (GTK_TABLE (data->parent), label, 0, 1, row, row+1, 0, 0, 0, 0);
	gtk_table_attach (GTK_TABLE (data->parent), hbox, 1, 2, row, row+1, GTK_FILL|GTK_EXPAND, GTK_FILL, 0, 0);

	g_signal_connect (url_entry, "changed", G_CALLBACK(url_entry_changed), data->config);

	if (server_url)
		gtk_entry_set_text(GTK_ENTRY (url_entry), server_url);
	g_object_set_data ((GObject *)button, "url-entry", (gpointer) url_entry);
	g_signal_connect (button, "clicked", G_CALLBACK(discover_server_url), data->config);

	/* if the host is null, then user+other info is dropped silently, force it to be kept */
	if (!server_url|| !*server_url) {
		gchar *uri;

		uri = g_strdup_printf (EVOLUTION_ACCOUNT_URL_FORMAT, address, address, "");
		e_account_set_string(target_account->MODIFIED_ACCT,
				     E_ACCOUNT_SOURCE_URL, uri);
		g_free(uri);
	}

	g_free (server_url);
	g_free (username);

	return hbox;
}
#endif

#if EDS_CHECK_VERSION (3,3,90)
gboolean
org_gnome_activesync_check_options(EPlugin *epl, EConfigHookPageCheckData *data)
{
	EMConfigTargetSettings *target = (EMConfigTargetSettings *)(data->config->target);
	CamelEasSettings *eas_settings;
	gint status = TRUE;

	if (!CAMEL_IS_EAS_SETTINGS (target->storage_settings))
		return TRUE;

	eas_settings = CAMEL_EAS_SETTINGS (target->storage_settings);

	if (data->pageid == NULL ||
	    strcmp (data->pageid, "10.receive") == 0 ||
	    strcmp (data->pageid, "20.receive_options") == 0) {
		char *url;
		char *key;
		const char *address = target->email_address;
		GConfClient *client = gconf_client_get_default ();

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
		url = gconf_client_get_string (client, key, NULL);
		g_object_unref (client);
		g_free (key);
	
			/* Note: we only care about activesync url's, we WILL get called on all other url's too. */
		if ((!url|| !*url)
		    && strcmp(target->storage_protocol, "eas") == 0) {
			status = FALSE;
		} 		

		g_free (url);
	}

	return status;
}

#else
gboolean
org_gnome_activesync_check_options(EPlugin *epl, EConfigHookPageCheckData *data)
{
	EMConfigTargetAccount *target = (EMConfigTargetAccount *)data->config->target;
	gint status = TRUE;

	if (data->pageid == NULL ||
	    strcmp (data->pageid, "10.receive") == 0 ||
	    strcmp (data->pageid, "20.receive_options") == 0) {
		char *url;
		char *key;
		const char *address = e_account_get_string (target->MODIFIED_ACCT,
							    E_ACCOUNT_ID_ADDRESS);
		GConfClient *client = gconf_client_get_default ();
		CamelURL *curl;
		const gchar * target_url = e_account_get_string(target->MODIFIED_ACCT,
								E_ACCOUNT_SOURCE_URL);

		curl = camel_url_new(target_url, NULL);
		if (!curl)
			return FALSE;

		key = g_strdup_printf ("/apps/activesyncd/accounts/%s/serverUri", address);
		url = gconf_client_get_string (client, key, NULL);
		g_object_unref (client);
		g_free (key);
	
			/* Note: we only care about activesync url's, we WILL get called on all other url's too. */
		if ((!url|| !*url)
		    && strcmp(curl->protocol, "eas") == 0) {
			status = FALSE;
		} 		

		g_free (url);
	}

	return status;
}
#endif

#if EDS_CHECK_VERSION (3,3,90)
void 
org_gnome_activesync_commit (EPlugin *epl, EMConfigTargetSettings *target_account)
{
	if (!CAMEL_IS_EAS_SETTINGS (target_account->storage_settings))
		return;

	/* Verify the storage and transport settings are shared. */
	g_warn_if_fail (
		target_account->storage_settings ==
		target_account->transport_settings);

}
#else
void
org_gnome_activesync_commit (EPlugin *epl, EMConfigTargetAccount *target_account)
{
	const gchar *source_url;
	CamelURL *url;

	printf("\n\n\n\n*********************************\n\n\n");
	source_url = e_account_get_string (target_account->MODIFIED_ACCT,
					   E_ACCOUNT_SOURCE_URL);
	if (source_url && source_url[0] != '\0')
		url = camel_url_new (source_url, NULL);
	else
		url = NULL;

	if (url == NULL
	    || strcmp (url->protocol, "eas") != 0) {
		if (url)
			camel_url_free (url);

		return;
	}

	camel_url_free (url);

	return;
}
#endif

static void
free_eas_account_listener ( void )
{
	g_object_unref (config_listener);
}

gint
e_plugin_lib_enable (EPlugin *ep, gint enable)
{
	if (!config_listener) {
		config_listener = eas_account_listener_new ();
		g_atexit ( free_eas_account_listener );
	}

	return 0;
}
