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
 *		Srinivasa Ragavan <sragavan@gnome.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eas-account-listener.h"
#include <string.h>
#include <glib/gi18n.h>
#include <camel/camel.h>
#include <libedataserver/e-account.h>
#include <libecal/e-cal.h>
#include <shell/e-shell.h>

static	GList *eas_accounts = NULL;

struct _EasAccountListenerPrivate {
	GConfClient *gconf_client;
	/* we get notification about mail account changes form this object */
	EAccountList *account_list;
};

struct _EasAccountInfo {
	gchar *uid;
	gchar *name;
	gchar *source_url;
};

typedef struct _EasAccountInfo EasAccountInfo;

#define EAS_URI_PREFIX   "eas://"
#define EAS_PREFIX_LENGTH 6

#define PARENT_TYPE G_TYPE_OBJECT

static GObjectClass *parent_class = NULL;

static void dispose (GObject *object);
static void finalize (GObject *object);

static void
eas_account_listener_class_init (EasAccountListenerClass *class)
{
	GObjectClass *object_class;

	parent_class =  g_type_class_ref (PARENT_TYPE);
	object_class = G_OBJECT_CLASS (class);

	/* virtual method override */
	object_class->dispose = dispose;
	object_class->finalize = finalize;
}

static void
eas_account_listener_init (EasAccountListener *config_listener,  EasAccountListenerClass *class)
{
	config_listener->priv = g_new0 (EasAccountListenerPrivate, 1);
}

static void
dispose (GObject *object)
{
	EasAccountListener *config_listener = EAS_ACCOUNT_LISTENER (object);

	g_object_unref (config_listener->priv->gconf_client);
	g_object_unref (config_listener->priv->account_list);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
finalize (GObject *object)
{
	EasAccountListener *config_listener = EAS_ACCOUNT_LISTENER (object);
	GList *list;
	EasAccountInfo *info;

	if (config_listener->priv) {
		g_free (config_listener->priv);
	}

	for (list = g_list_first (eas_accounts); list; list = g_list_next (list)) {

		info = (EasAccountInfo *) (list->data);

		if (info) {

			g_free (info->uid);
			g_free (info->name);
			g_free (info->source_url);
			g_free (info);
		}
	}

	g_list_free (eas_accounts);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*determines whehter the passed in account is eas or not by looking at source url */

static gboolean
is_eas_account (EAccount *account)
{
	if (account->source->url != NULL) {
		return (strncmp (account->source->url,  EAS_URI_PREFIX, EAS_PREFIX_LENGTH ) == 0);
	} else {
		return FALSE;
	}
}

/* looks up for an existing eas account info in the eas_accounts list based on uid */

static EasAccountInfo*
lookup_account_info (const gchar *key)
{
	GList *list;
        EasAccountInfo *info;
	gint found = 0;

        if (!key)
                return NULL;

	info = NULL;

        for (list = g_list_first (eas_accounts);  list;  list = g_list_next (list)) {
                info = (EasAccountInfo *) (list->data);
                found = (strcmp (info->uid, key) == 0);
		if (found)
			break;
	}
	if (found)
		return info;
	return NULL;
}

#define CALENDAR_SOURCES "/apps/evolution/calendar/sources"
#define TASKS_SOURCES "/apps/evolution/tasks/sources"
#define NOTES_SOURCES "/apps/evolution/memos/sources"
#define SELECTED_CALENDARS "/apps/evolution/calendar/display/selected_calendars"
#define SELECTED_TASKS   "/apps/evolution/calendar/tasks/selected_tasks"
#define SELECTED_NOTES   "/apps/evolution/calendar/memos/selected_memos"

static char *
add_addressbook_sources (EAccount *account)
{
	ESourceList *list;
        ESourceGroup *group;
        ESource *source;
	GList *books_list;
	GConfClient* client;
	char *uid;

	client = gconf_client_get_default ();
	list = e_source_list_new_for_gconf (client, "/apps/evolution/addressbook/sources" );
	group = e_source_group_new (e_account_get_string (account, E_ACCOUNT_ID_ADDRESS), "local:");
	
	source = e_source_new (_("Contacts"), e_account_get_string (account, E_ACCOUNT_ID_ADDRESS));
	
	e_source_set_property (source, "completion", "true");
	e_source_group_add_source (group, source, -1);
	uid = g_strdup(e_source_peek_uid(source));

	e_source_list_add_group (list, group, -1);
	e_source_list_sync (list, NULL);
	g_object_unref (group);
	g_object_unref (list);
	g_object_unref (client);

	return uid;
}

static char *
add_calendar_sources (EAccount *account)
{
	ESourceList *list;
        ESourceGroup *group;
        ESource *source;
	GList *books_list;
	GConfClient* client;
	char *uid;
	GSList *ids, *temp;

	client = gconf_client_get_default ();
	list = e_source_list_new_for_gconf (client, CALENDAR_SOURCES );
	group = e_source_group_new (e_account_get_string (account, E_ACCOUNT_ID_ADDRESS), "local:");
	
	source = e_source_new (_("Calendar"), e_account_get_string (account, E_ACCOUNT_ID_ADDRESS));
	uid = g_strdup(e_source_peek_uid(source));
	e_source_group_add_source (group, source, -1);

	e_source_list_add_group (list, group, -1);
	e_source_list_sync (list, NULL);

	ids = gconf_client_get_list (client, SELECTED_CALENDARS, GCONF_VALUE_STRING, NULL);
	ids = g_slist_append (ids, g_strdup (e_source_peek_uid (source)));
	gconf_client_set_list (client,  SELECTED_CALENDARS, GCONF_VALUE_STRING, ids, NULL);
	temp  = ids;

	for (; temp != NULL; temp = g_slist_next (temp))
		g_free (temp->data);

	g_slist_free (ids);

	g_object_unref (source);
	g_object_unref (group);
	g_object_unref (list);
	g_object_unref (client);

	return uid;
}

static void
remove_calendar_sources (EAccount *account)
{
	ESourceList *list;
        GSList *groups;
	gboolean found_group;
	GConfClient* client;
	GSList *ids, *temp;
	GSList *node_tobe_deleted;
	const gchar *source_selection_key;
	const char *group_name = e_account_get_string (account, E_ACCOUNT_ID_ADDRESS);

        client = gconf_client_get_default();
        list = e_source_list_new_for_gconf (client, CALENDAR_SOURCES);
	groups = e_source_list_peek_groups (list);
	
	found_group = FALSE;

	for (; groups != NULL && !found_group; groups = g_slist_next (groups)) {
		ESourceGroup *group = E_SOURCE_GROUP (groups->data);

		if (strcmp (e_source_group_peek_name (group), group_name) == 0 &&
		   strcmp (e_source_group_peek_base_uri (group), "local:") == 0) {
			GSList *sources = e_source_group_peek_sources (group);

			for (; sources != NULL; sources = g_slist_next (sources)) {
				ESource *source = E_SOURCE (sources->data);
				const gchar *source_relative_uri;

				source_relative_uri = e_source_peek_relative_uri (source);
				if (source_relative_uri == NULL)
					continue;
				if (strcmp (source_relative_uri, group_name) == 0) {

					printf("CAL URI: %s\n", e_source_get_uri(source));
					ids = gconf_client_get_list (client, SELECTED_CALENDARS,
								     GCONF_VALUE_STRING, NULL);
					node_tobe_deleted = g_slist_find_custom (ids, e_source_peek_uid (source), (GCompareFunc) strcmp);
					if (node_tobe_deleted) {
						g_free (node_tobe_deleted->data);
						ids = g_slist_delete_link (ids, node_tobe_deleted);
					}
					gconf_client_set_list (client,  SELECTED_CALENDARS,
							       GCONF_VALUE_STRING, ids, NULL);
					temp  = ids;
					for (; temp != NULL; temp = g_slist_next (temp))
						g_free (temp->data);
					g_slist_free (ids);

					e_source_list_remove_group (list, group);
					e_source_list_sync (list, NULL);
					found_group = TRUE;
					break;

				}
			}

		}

	}

	g_object_unref (list);
	g_object_unref (client);
	
}
static void
remove_addressbook_sources (EAccount *account)
{
	ESourceList *list;
        ESourceGroup *group;
	GSList *groups;
	gboolean found_group;
	GConfClient *client;


	client = gconf_client_get_default ();
	list = e_source_list_new_for_gconf (client, "/apps/evolution/addressbook/sources" );
	groups = e_source_list_peek_groups (list);

	found_group = FALSE;

	for (; groups != NULL &&  !found_group; groups = g_slist_next (groups)) {

		group = E_SOURCE_GROUP (groups->data);
		if ( strcmp ( e_source_group_peek_base_uri (group), "local:") == 0 && strcmp (e_source_group_peek_name (group), e_account_get_string (account, E_ACCOUNT_ID_ADDRESS)) == 0) {

			e_source_list_remove_group (list, group);
			e_source_list_sync (list, NULL);
			found_group = TRUE;

		}
	}
	g_object_unref (list);
	g_object_unref (client);

}

static char *
sanitize_email (const char *email)
{
	char *id = g_strdup(email);
	int i, len = strlen (id);

	for (i=0; i<len; i++) {
		if (id[i] == '@')
			id[i] = '_';
		else if (id[i] == '.')
			id[i] = '_';
	}

	return id;
}


/* Sample commands 
 * 
 * syncevolution --configure syncURL= username=meegotabletmail@gmail.com addressbook/backend=eas-contacts calendar/backend=eas-events  target-config@meegotabletmail_gmail_com addressbook calendar 
 * syncevolution --configure calendar/database=local:meegotabletmail@gmail.com addressbook/database=local:meegotabletmail@gmail.com --template SyncEvolution_Client syncURL=local://@meegotabletmail_gmail_com username= password= meegotabletmail_gmail_com calendar addressbook 
 *
 */


#define SYNCEVOLUTION_SETUP_1 "syncevolution --configure syncURL= username=%s addressbook/backend=eas-contacts calendar/backend=eas-events  target-config@%s addressbook calendar"
#define SYNCEVOLUTION_SETUP_2 "syncevolution --configure calendar/database=local:%s addressbook/database=local:%s --template SyncEvolution_Client syncURL=local://@%s username= password= %s calendar addressbook"
#define SYNCEVOLUTION_UNDO "syncevolution --remove %s%s"

static void
link_syncevolution (EAccount *account)
{
	char *config_id;
	const char *email = e_account_get_string (account, E_ACCOUNT_ID_ADDRESS);
	char *command;
	int exit_status = 0;
	gboolean ret;

	config_id = sanitize_email (email);

	command = g_strdup_printf(SYNCEVOLUTION_SETUP_1, email, config_id);
	g_debug ("Executing: %s\n", command);
	ret = g_spawn_command_line_sync (command, NULL, NULL, &exit_status, NULL);
	g_free (command);
	if (!ret || exit_status != 0) {
		g_warning ("Unable to execute syncevolution command1\n");
		return;
	}

	command = g_strdup_printf(SYNCEVOLUTION_SETUP_2, email, email, config_id, config_id);
	g_debug ("Executing: %s\n", command);
	ret = g_spawn_command_line_sync (command, NULL, NULL, &exit_status, NULL);
	if (!ret || exit_status != 0) {
		g_warning ("Unable to execute syncevolution command2\n");
		return;	
	}

	g_free (config_id);
}

static void
unlink_syncevolution (EAccount *account)
{
	char *config_id;
	const char *email = e_account_get_string (account, E_ACCOUNT_ID_ADDRESS);
	char *command;
	int exit_status = 0;
	gboolean ret;

	config_id = sanitize_email (email);

	command = g_strdup_printf(SYNCEVOLUTION_UNDO, config_id, "");
	g_debug ("Executing: %s\n", command);
	ret = g_spawn_command_line_sync (command, NULL, NULL, &exit_status, NULL);
	g_free (command);
	if (!ret || exit_status != 0) {
		g_warning ("Unable to execute syncevolution command1\n");
		return;
	}

	command = g_strdup_printf(SYNCEVOLUTION_UNDO, "target-config@", config_id);
	g_debug ("Executing: %s\n", command);
	ret = g_spawn_command_line_sync (command, NULL, NULL, &exit_status, NULL);
	if (!ret || exit_status != 0) {
		g_warning ("Unable to execute syncevolution command2\n");
		return;	
	}
	g_free (config_id);
}

static void
account_added (EAccountList *account_listener, EAccount *account)
{

	EasAccountInfo *info;
	EAccount *parent;
	gboolean status;
	CamelURL *parent_url;
	char *book_uid, *cal_uid;

	if (!is_eas_account (account))
		return;

	info = g_new0 (EasAccountInfo, 1);
	info->uid = g_strdup (account->uid);
	info->name = g_strdup (account->name);
	info->source_url = g_strdup (account->source->url);

	book_uid = add_addressbook_sources (account);
	cal_uid = add_calendar_sources (account);

	eas_accounts = g_list_append (eas_accounts, info);
	link_syncevolution (account);
}

static void
account_removed (EAccountList *account_listener, EAccount *account)
{
	EasAccountInfo *info;

	if (!is_eas_account (account))
		return;

	info = lookup_account_info (account->uid);
	if (info == NULL)
		return;

	remove_calendar_sources (account);
	remove_addressbook_sources (account);
	unlink_syncevolution (account);

	eas_accounts = g_list_remove (eas_accounts, info);
	g_free (info->uid);
	g_free (info->name);
	g_free (info->source_url);
        g_free (info);
}

static void
account_changed (EAccountList *account_listener, EAccount *account)
{
	gboolean is_account;
	CamelURL *old_url, *new_url;
	EasAccountInfo *existing_account_info;

	is_account = is_eas_account (account);

	existing_account_info = lookup_account_info (account->uid);

	if (existing_account_info == NULL && is_account) {

		if (!account->enabled)
			return;

		/* some account of other type is changed to Groupwise */
		account_added (account_listener, account);

	} else if ( existing_account_info != NULL && !is_account) {

		/* Active Sync account is changed to some other type */
		remove_calendar_sources (account);
		remove_addressbook_sources (account);
		unlink_syncevolution (account);

		eas_accounts = g_list_remove (eas_accounts, existing_account_info);
		g_free (existing_account_info->uid);
		g_free (existing_account_info->name);
		g_free (existing_account_info->source_url);
		g_free (existing_account_info);

	} else if (existing_account_info != NULL && is_account) {

		if (!account->enabled) {
			account_removed (account_listener, account);
			return;
		}
	}
}

static void
eas_account_listener_construct (EasAccountListener *config_listener)
{
	EIterator *iter;
	EAccount *account;
	EasAccountInfo *info;

	config_listener->priv->account_list = e_account_list_new (config_listener->priv->gconf_client);

	for (iter = e_list_get_iterator (E_LIST ( config_listener->priv->account_list) ); e_iterator_is_valid (iter); e_iterator_next (iter)) {

		account = E_ACCOUNT (e_iterator_get (iter));

		if ( is_eas_account (account) && account->enabled) {

			info = g_new0 (EasAccountInfo, 1);
			info->uid = g_strdup (account->uid);
			info->name = g_strdup (account->name);
			info->source_url = g_strdup (account->source->url);
			eas_accounts = g_list_append (eas_accounts, info);

		}

	}

	g_signal_connect (config_listener->priv->account_list, "account_added", G_CALLBACK (account_added), NULL);
	g_signal_connect (config_listener->priv->account_list, "account_changed", G_CALLBACK (account_changed), NULL);
	g_signal_connect (config_listener->priv->account_list, "account_removed", G_CALLBACK (account_removed), NULL);
}

GType
eas_account_listener_get_type (void)
{
	static GType eas_account_listener_type  = 0;

	if (!eas_account_listener_type) {
		static GTypeInfo info = {
                        sizeof (EasAccountListenerClass),
                        (GBaseInitFunc) NULL,
                        (GBaseFinalizeFunc) NULL,
                        (GClassInitFunc) eas_account_listener_class_init,
                        NULL, NULL,
                        sizeof (EasAccountListener),
                        0,
                        (GInstanceInitFunc) eas_account_listener_init
                };
		eas_account_listener_type = g_type_register_static (PARENT_TYPE, "EasAccountListener", &info, 0);
	}

	return eas_account_listener_type;
}

EasAccountListener*
eas_account_listener_new (void)
{
	EasAccountListener *config_listener;

	config_listener = g_object_new (EAS_TYPE_ACCOUNT_LISTENER, NULL);
	config_listener->priv->gconf_client = gconf_client_get_default();

	eas_account_listener_construct (config_listener);

	return config_listener;
}
