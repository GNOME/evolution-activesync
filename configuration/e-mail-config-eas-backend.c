/*
 * e-mail-config-eas-backend.c
 * The configuration UI runs in Evolution.
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
 *
 * Authors:
 *		Oliver Luo <lyc.pku.eecs@gmail.com>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "e-mail-config-eas-backend.h"

#include <glib/gi18n-lib.h>
#include <gio/gio.h>

#include <camel/camel.h>
#include <libebackend/libebackend.h>
#include <libeasmail.h>

#include <mail/e-mail-config-auth-check.h>
#include <mail/e-mail-config-receiving-page.h>

/* To use camel_eas_settings_set_account_uid() */
#include "libevoeas/camel-eas-settings.h"

#define E_MAIL_CONFIG_EAS_BACKEND_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), E_TYPE_MAIL_CONFIG_EAS_BACKEND, EMailConfigEasBackendPrivate))

struct _EMailConfigEasBackendPrivate {
	GtkWidget *user_entry;		/* not referenced */
	GtkWidget *host_entry;		/* not referenced */
	GtkWidget *auth_check;		/* not referenced */
	GtkWidget *autodiscover_button;
};

G_DEFINE_DYNAMIC_TYPE (
	EMailConfigEasBackend,
	e_mail_config_eas_backend,
	E_TYPE_MAIL_CONFIG_SERVICE_BACKEND)

static ESource *
mail_config_eas_backend_new_collection (EMailConfigServiceBackend *backend)
{

	EMailConfigServiceBackendClass *class;
	ESourceBackend *extension;
	ESource *source;
	const gchar *extension_name;

	/* This backend serves double duty.  One instance holds the
	 * mail account source, another holds the mail transport source.
	 * We can differentiate by examining the EMailConfigServicePage
	 * the backend is associated with.  We return a new collection
	 * for both the Receiving Page and Sending Page.  Although the
	 * Sending Page instance ultimately gets discarded, it's still
	 * needed to avoid creating an [Eas Backend] extension in the
	 * mail transport source. */

	class = E_MAIL_CONFIG_SERVICE_BACKEND_GET_CLASS (backend);

	source = e_source_new (NULL, NULL, NULL);
	extension_name = E_SOURCE_EXTENSION_COLLECTION;
	extension = e_source_get_extension (source, extension_name);
	e_source_backend_set_backend_name (extension, class->backend_name);

	return source;
}

/* Server URL auto discover button handler */
static void
discover_server_url (GtkWidget *button, EMailConfigServiceBackend *backend)
{
	EasEmailHandler *handler;
	GError *error = NULL;
	EMailConfigServicePage *page;
	const gchar * email_address;
	gchar *uri = NULL;
	gchar *username;
	GtkWidget *username_entry = (GtkWidget *)g_object_get_data ((GObject *)button, "username-entry");
	GtkWidget *host_entry = (GtkWidget *)g_object_get_data ((GObject *)button, "url-entry");

	page = e_mail_config_service_backend_get_page (backend);
	email_address = e_mail_config_service_page_get_email_address (page);

	if (email_address != NULL) {
		handler = eas_mail_handler_new(email_address, &error);
		if (error) {
			g_warning ("Unable to create mailHandler. We don't suppport auto-discover: %s\n", error->message);
			g_error_free (error);
			gtk_widget_set_sensitive (button, FALSE);
			return;
		}

		username = g_strdup (gtk_entry_get_text((GtkEntry *)username_entry));
		if (username == NULL || *username == '\0' || strcmp (username, email_address) == 0) {
			g_free (username);
			username = NULL;
		}

		/* Core server URL auto discover handler */
		eas_mail_handler_autodiscover(
			handler,
			email_address,
			username,
			&uri,
			NULL,
			&error);

		if (!error && uri && uri[0])
			gtk_entry_set_text ((GtkEntry *)host_entry, uri);

		g_free (username);
		g_object_unref (handler);
	}
}

/* Construct the Receiving Email page. */
static void
mail_config_eas_backend_insert_widgets (EMailConfigServiceBackend *backend,
                                        GtkBox *parent)
{
	EMailConfigEasBackendPrivate *priv;
	EMailConfigServicePage *page;
	ESource *source;
	ESourceExtension *extension;
	CamelSettings *settings;
	GtkLabel *label;
	GtkWidget *widget;
	GtkWidget *container;
	const gchar *extension_name;
	const gchar *text;
	gchar *markup;

	priv = E_MAIL_CONFIG_EAS_BACKEND_GET_PRIVATE (backend);
	page = e_mail_config_service_backend_get_page (backend);

	/* This backend serves double duty.  One instance holds the
	 * mail account source, another holds the mail transport source.
	 * We can differentiate by examining the EMailConfigServicePage
	 * the backend is associated with.  This method only applies to
	 * the Receiving Page. */
	if (!E_IS_MAIL_CONFIG_RECEIVING_PAGE (page))
		return;

	/* This needs to come _after_ the page type check so we don't
	 * introduce a backend extension in the mail transport source. */
	settings = e_mail_config_service_backend_get_settings (backend);

	/* Add widget. */
	text = _("Configuration");
	markup = g_markup_printf_escaped ("<b>%s</b>", text);
	widget = gtk_label_new (markup);
	gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
	gtk_box_pack_start (GTK_BOX (parent), widget, FALSE, FALSE, 0);
	gtk_widget_show (widget);
	g_free (markup);

	widget = gtk_grid_new ();
	gtk_widget_set_margin_start (widget, 12);
	gtk_grid_set_row_spacing (GTK_GRID (widget), 6);
	gtk_grid_set_column_spacing (GTK_GRID (widget), 6);
	gtk_box_pack_start (GTK_BOX (parent), widget, FALSE, FALSE, 0);
	gtk_widget_show (widget);

	container = widget;

	widget = gtk_label_new_with_mnemonic (_("User_name:"));
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (container), widget, 0, 0, 1, 1);
	gtk_widget_show (widget);

	label = GTK_LABEL (widget);

	widget = gtk_entry_new ();
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_label_set_mnemonic_widget (label, widget);
	gtk_grid_attach (GTK_GRID (container), widget, 1, 0, 2, 1);
	priv->user_entry = widget;  /* do not reference */
	gtk_widget_show (widget);

	widget = gtk_label_new_with_mnemonic (_("_Server URL:"));
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (container), widget, 0, 1, 1, 1);
	gtk_widget_show (widget);

	label = GTK_LABEL (widget);

	widget = gtk_entry_new ();
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_label_set_mnemonic_widget (label, widget);
	gtk_grid_attach (GTK_GRID (container), widget, 1, 1, 1, 1);
	priv->host_entry = widget;  /* do not reference */
	gtk_widget_show (widget);
	
	text = _("Authentication");
	markup = g_markup_printf_escaped ("<b>%s</b>", text);
	widget = gtk_label_new (markup);
	gtk_widget_set_margin_top (widget, 6);
	gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
	gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_widget_set_valign (widget, GTK_ALIGN_CENTER);
	gtk_box_pack_start (GTK_BOX (parent), widget, FALSE, FALSE, 0);
	gtk_widget_show (widget);
	g_free (markup);

	widget = e_mail_config_auth_check_new (backend);
	gtk_widget_set_margin_start (widget, 12);
	gtk_box_pack_start (GTK_BOX (parent), widget, FALSE, FALSE, 0);
	priv->auth_check = widget;  /* do not reference */
	gtk_widget_show (widget);

	widget = gtk_button_new_with_mnemonic (_("_Auto Detect"));
	gtk_grid_attach(GTK_GRID (container), widget, 2, 1, 1, 1);
	priv->autodiscover_button = widget;
	gtk_widget_show (widget);

	/* Connect auto discover button to it's handler with information needed. */
	g_object_set_data ((GObject *)widget, "username-entry", (gpointer)priv->user_entry);
	g_object_set_data ((GObject *)widget, "url-entry", (gpointer)priv->host_entry);
	g_signal_connect (widget, "clicked", G_CALLBACK(discover_server_url), backend);

	/* Bind property in priv with widget to automatically read the input. */
	g_object_bind_property (
		settings, "user",
		priv->user_entry, "text",
		G_BINDING_BIDIRECTIONAL |
		G_BINDING_SYNC_CREATE);

	g_object_bind_property (
		settings, "host",
		priv->host_entry, "text",
		G_BINDING_BIDIRECTIONAL |
		G_BINDING_SYNC_CREATE);
	
	/* Don't use G_BINDING_SYNC_CREATE here since the widget
	 * chooses its initial mechanism more intelligently than
	 * a simple property binding would. */
	g_object_bind_property (
		settings, "auth-mechanism",
		priv->auth_check, "active-mechanism",
		G_BINDING_BIDIRECTIONAL);

	extension_name = E_SOURCE_EXTENSION_COLLECTION;
	source = e_mail_config_service_backend_get_collection (backend);
	extension = e_source_get_extension (source, extension_name);

	/* The collection identity is the user name. */
	g_object_bind_property (
		settings, "user",
		extension, "identity",
		G_BINDING_BIDIRECTIONAL |
		G_BINDING_SYNC_CREATE);
}

/* Setup default value in the Receiving page using the information Stored 
 * in GSettings. */
static void
mail_config_eas_backend_setup_defaults (EMailConfigServiceBackend *backend)
{
	CamelSettings *settings;
	EMailConfigServicePage *page;
	const gchar *email_address;
	gchar *username;
	gchar *hosturl;

	page = e_mail_config_service_backend_get_page (backend);

	/* This backend serves double duty.  One instance holds the
	 * mail account source, another holds the mail transport source.
	 * We can differentiate by examining the EMailConfigServicePage
	 * the backend is associated with.  This method only applies to
	 * the Receiving Page. */
	if (!E_IS_MAIL_CONFIG_RECEIVING_PAGE (page))
		return;

	/* This needs to come _after_ the page type check so we don't
	 * introduce a backend extension in the mail transport source. */
	settings = e_mail_config_service_backend_get_settings (backend);

	email_address = e_mail_config_service_page_get_email_address (page);

	if (email_address != NULL) {
		CamelNetworkSettings *network_settings;
		gchar *account_address = g_strdup_printf ("/org/meego/activesyncd/account/%s/", email_address);
		
		g_debug("Path is %s\n", account_address);
		
		GSettings *account = g_settings_new_with_path ("org.meego.activesyncd.account", account_address);
		
		g_free (account_address);
	
		/* The default username is the same as the email address. */
		username = g_settings_get_string (account, "username");

		if (username == NULL || *username == '\0') {
			username = g_strdup (email_address);
		}

		hosturl = g_settings_get_string (account, "serveruri");

		network_settings = CAMEL_NETWORK_SETTINGS (settings);
		camel_network_settings_set_user (network_settings, username);
		if (hosturl && hosturl[0]) {
			camel_network_settings_set_host (network_settings, hosturl);
		}

		g_free (username);
		g_free (hosturl);
		g_object_unref (account);
	}
}

/* Check whether the information is enough to complete current step. */
static gboolean
mail_config_eas_backend_check_complete (EMailConfigServiceBackend *backend)
{
	EMailConfigServicePage *page;
	CamelSettings *settings;
	CamelNetworkSettings *network_settings;
	const gchar *username;
	const gchar *hosturl;

	page = e_mail_config_service_backend_get_page (backend);

	/* This backend serves double duty.  One instance holds the
	 * mail account source, another holds the mail transport source.
	 * We can differentiate by examining the EMailConfigServicePage
	 * the backend is associated with.  This method only applies to
	 * the Receiving Page. */
	if (!E_IS_MAIL_CONFIG_RECEIVING_PAGE (page))
		return TRUE;

	/* This needs to come _after_ the page type check so we don't
	 * introduce a backend extension in the mail transport source. */
	settings = e_mail_config_service_backend_get_settings (backend);

	network_settings = CAMEL_NETWORK_SETTINGS (settings);
	username = camel_network_settings_get_user (network_settings);
	hosturl = camel_network_settings_get_host (network_settings);

	if (username == NULL || *username == '\0')
		return FALSE;

	if (hosturl == NULL || *hosturl == '\0')
		return FALSE;

	return TRUE;
}

/* Save the filled information to GSettings and move on. */
static void
mail_config_eas_backend_commit_changes (EMailConfigServiceBackend *backend)
{
	CamelSettings *settings;
	EMailConfigServicePage *page;
	const gchar *email_address;
	gchar *username;
	gchar *hosturl;
	
	page = e_mail_config_service_backend_get_page (backend);

	/* This backend serves double duty.  One instance holds the
	 * mail account source, another holds the mail transport source.
	 * We can differentiate by examining the EMailConfigServicePage
	 * the backend is associated with.  This method only applies to
	 * the Receiving Page. */
	if (!E_IS_MAIL_CONFIG_RECEIVING_PAGE (page))
		return;

	/* This needs to come _after_ the page type check so we don't
	 * introduce a backend extension in the mail transport source. */
	settings = e_mail_config_service_backend_get_settings (backend);

	email_address = e_mail_config_service_page_get_email_address (page);

	if (email_address != NULL) {
		int i = 0;
		GSettings *account_info = g_settings_new ("org.meego.activesyncd");
		gchar **accounts = g_settings_get_strv(account_info, "accounts");
		gchar *account_address = g_strdup_printf ("/org/meego/activesyncd/account/%s/", email_address);

		g_debug("Path is %s\n", account_address);

		GSettings *account = g_settings_new_with_path ("org.meego.activesyncd.account", account_address);
		
		g_free (account_address);

		username = camel_network_settings_dup_user ((CamelNetworkSettings *)settings);
		g_strstrip (username);

		if (username && username[0]) {
			g_settings_set_string (account, "username", username);
			if (!g_strv_contains((const gchar * const *)accounts, email_address)){
				gchar **new_strv = g_malloc0(sizeof(gchar *) * (g_strv_length(accounts) + 2));
				for (i = 0; i < g_strv_length(accounts); i++)
					new_strv[i] = accounts[i];
				new_strv[g_strv_length(accounts)] = strdup(email_address);
				g_settings_set_strv(account_info, "accounts", (const gchar * const *)new_strv);
			}
		}

		g_free(username);

		hosturl = camel_network_settings_dup_host ((CamelNetworkSettings *)settings);
		g_strstrip (hosturl);

		if (hosturl && hosturl[0]) {
			g_settings_set_string (account, "serveruri", hosturl);	
			if (!g_strv_contains((const gchar * const *)accounts, email_address)){
				gchar **new_strv = g_malloc0(sizeof(gchar *) * (g_strv_length(accounts) + 2));
				for (i = 0; i < g_strv_length(accounts); i++)
					new_strv[i] = accounts[i];
				new_strv[g_strv_length(accounts)] = strdup(email_address);
				g_settings_set_strv(account_info, "accounts", (const gchar * const *)new_strv);
			}

			camel_eas_settings_set_account_uid ((CamelEasSettings *)settings, email_address);
		}

		g_debug("Sync now\n");

		g_settings_sync();

		g_free(hosturl);
		g_object_unref (account_info);
		g_object_unref (account);
	}
}

static void
e_mail_config_eas_backend_class_init (EMailConfigEasBackendClass *class)
{
	EMailConfigServiceBackendClass *backend_class;

	g_type_class_add_private (
		class, sizeof (EMailConfigEasBackendPrivate));

	backend_class = E_MAIL_CONFIG_SERVICE_BACKEND_CLASS (class);
	backend_class->backend_name = "eas";
	backend_class->new_collection = mail_config_eas_backend_new_collection;
	backend_class->insert_widgets = mail_config_eas_backend_insert_widgets;
	backend_class->setup_defaults = mail_config_eas_backend_setup_defaults;
	backend_class->check_complete = mail_config_eas_backend_check_complete;
	backend_class->commit_changes = mail_config_eas_backend_commit_changes;
}

static void
e_mail_config_eas_backend_class_finalize (EMailConfigEasBackendClass *class)
{
}

static void
e_mail_config_eas_backend_init (EMailConfigEasBackend *backend)
{
	backend->priv = E_MAIL_CONFIG_EAS_BACKEND_GET_PRIVATE (backend);
}

void
e_mail_config_eas_backend_type_register (GTypeModule *type_module)
{
	/* XXX G_DEFINE_DYNAMIC_TYPE declares a static type registration
	 *     function, so we have to wrap it with a public function in
	 *     order to register types from a separate compilation unit. */
	e_mail_config_eas_backend_register_type (type_module);
}
