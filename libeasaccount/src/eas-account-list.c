/*
 * ActiveSync account management
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Derived from code in Evolution's libedataserver library, marked:
 * Copyright © 1999-2008 Novell, Inc. (www.novell.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */

#include <glib.h>
#include <string.h>

#include "eas-account-list.h"
#include "eas-account.h"


struct _EasAccountListPrivate {
	GConfClient *gconf;
	guint notify_id;
};

enum {
	ACCOUNT_ADDED,
	ACCOUNT_CHANGED,
	ACCOUNT_REMOVED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void eas_account_list_dispose (GObject *);
static void eas_account_list_finalize (GObject *);

G_DEFINE_TYPE (EasAccountList, eas_account_list, E_TYPE_LIST)

static void
eas_account_list_class_init (EasAccountListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	g_debug("eas_account_list_class_init++");
	/* virtual method override */
	object_class->dispose = eas_account_list_dispose;
	object_class->finalize = eas_account_list_finalize;

	/* signals */
	signals[ACCOUNT_ADDED] =
		g_signal_new ("account-added",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EasAccountListClass, account_added),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1,
			      EAS_TYPE_ACCOUNT);
	signals[ACCOUNT_CHANGED] =
		g_signal_new ("account-changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EasAccountListClass, account_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1,
			      EAS_TYPE_ACCOUNT);
	signals[ACCOUNT_REMOVED] =
		g_signal_new ("account-removed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EasAccountListClass, account_removed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__OBJECT,
			      G_TYPE_NONE, 1,
			      EAS_TYPE_ACCOUNT);
	g_debug("eas_account_list_class_init--");
}

static void
eas_account_list_init (EasAccountList *account_list)
{
	account_list->priv = g_new0 (EasAccountListPrivate, 1);
}

static void
eas_account_list_dispose (GObject *object)
{
	EasAccountList *account_list = EAS_ACCOUNT_LIST (object);
	g_debug("eas_account_list_dispose++");
	if (account_list->priv->gconf) {
		if (account_list->priv->notify_id) {
			gconf_client_notify_remove (account_list->priv->gconf,
						    account_list->priv->notify_id);
		}
		g_object_unref (account_list->priv->gconf);
		account_list->priv->gconf = NULL;
	}
	g_debug("eas_account_list_dispose--");
	G_OBJECT_CLASS (eas_account_list_parent_class)->dispose (object);
}

static void
eas_account_list_finalize (GObject *object)
{
	EasAccountList *account_list = EAS_ACCOUNT_LIST (object);
	g_debug("eas_account_list_finalize++");
	g_free (account_list->priv);
	g_debug("eas_account_list_finalize--");
	G_OBJECT_CLASS (eas_account_list_parent_class)->finalize (object);
}

static gchar*
get_key_absolute_path(const gchar *uid, const gchar *Key)
{
	int string_Key_len;
	char* Key_path = NULL;
	
	string_Key_len = strlen(EAS_ACCOUNT_ROOT) + strlen("/") + strlen(uid) + strlen(Key) + 1;
	
	Key_path = g_malloc(string_Key_len);
	if (Key_path)
		g_snprintf(Key_path, (string_Key_len), "%s/%s%s%c", EAS_ACCOUNT_ROOT, uid, Key, '\0');

	return Key_path;
} 

static void
eas_account_list_set_account_info(EasAccountInfo *acc_info, const gchar* uid_path, GSList *entry_list)
{
	const GConfValue* value = NULL;
	const gchar* keyname = NULL;
	gchar* uid = NULL;
	gint last_token;
	gchar **str_array = NULL; 
	gchar* serveruri_Key_path = NULL;
	gchar* username_Key_path = NULL;
	gchar* policy_key_Key_path = NULL;
	gchar* calendar_folder_Key_path = NULL;
	gchar* contact_folder_Key_path = NULL;
	gchar* password_Key_path = NULL;
	gchar* protover_Key_path = NULL;
	gchar* devover_Key_path = NULL;
	gchar* servoprotovers_Key_path = NULL;
	GSList *item = NULL;

	/* g_debug("eas_account_list_set_account_info++"); */
	g_return_if_fail (acc_info != NULL);
	g_return_if_fail (uid_path != NULL);
	g_return_if_fail (entry_list != NULL);

	/* strip the EAS_ACCOUNT_ROOT from the uid_path to get the uid only */
	last_token = 4;
	str_array = g_strsplit(uid_path, "/", -1);
	uid = g_strdup(str_array[last_token]);
	/* free the vector */
	g_strfreev(str_array);

	/* Concatenate "ROOT + UID + KEY" */
	serveruri_Key_path       = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVERURI);
	username_Key_path        = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_USERNAME);
	policy_key_Key_path      = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_POLICY_KEY);
	contact_folder_Key_path  = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CONTACT_FOLDER);
	calendar_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CALENDAR_FOLDER);
	password_Key_path        = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PASSWORD);
	protover_Key_path        = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PROTOCOL_VERSION);
	devover_Key_path         = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_DEVICE_ID);
	servoprotovers_Key_path  = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVER_PROTOCOLS);
	acc_info->uid = uid; // Ownership passed to the account into structure.

	for (item = entry_list; item; item = item->next)
	{
		GConfEntry *entry = item->data;

		keyname = gconf_entry_get_key(entry);
		if (keyname == NULL) {
			/* g_debug("Couldn't get the key name - this could be a delete notification!");*/
			continue;
		}

		value = gconf_entry_get_value(entry);
		if (value == NULL) {
			/*g_debug("Couldn't get the key value - this could be a delete notification!");*/
			continue;
		}

		// gconf_value_to_string passes memory ownership to the account into structure.
		if (strcmp(keyname, serveruri_Key_path) == 0) {
			acc_info->serverUri = gconf_value_to_string(value);
		} else if (strcmp(keyname, username_Key_path) == 0) {
			acc_info->username = gconf_value_to_string(value); 
		} else if (strcmp(keyname, policy_key_Key_path) == 0) {
			acc_info->policy_key = gconf_value_to_string(value);
		} else if (strcmp(keyname, calendar_folder_Key_path) == 0) {
			acc_info->calendar_folder = gconf_value_to_string(value);
		} else if (strcmp(keyname, contact_folder_Key_path) == 0) {
			acc_info->contact_folder = gconf_value_to_string(value);
		} else if (strcmp(keyname, password_Key_path) == 0) {
			acc_info->password = gconf_value_to_string(value); 
		} else if (strcmp(keyname, protover_Key_path) == 0) {
			acc_info->protocol_version = gconf_value_get_int(value);
		} else if (strcmp(keyname, devover_Key_path) == 0) {
			acc_info->device_id = gconf_value_to_string(value);
		} else if (strcmp(keyname, servoprotovers_Key_path) == 0) {
			GSList *list;
			//GConfValueType type;
			gint prot;
			guint i;

			//type = value->type;
			//g_assert(type == GCONF_VALUE_LIST);
			list = gconf_value_get_list(value);
			//type = gconf_value_get_list_type (value);
			//g_assert(type == GCONF_VALUE_INT);
			g_debug("list length = %d", g_slist_length(list));
			//g_debug("type of elements in list: %d", type);
			for (i = 0; i < g_slist_length(list); i++){	
				prot = GPOINTER_TO_INT (g_slist_nth_data(list, i));
				g_debug("prot = %d", prot);
				// lrm TODO - copy the list to account info
				//acc_info->server_protocols = g_slist_append(acc_info->server_protocols, g_slist_nth_data(list, i));
			}
		} else {
			g_warning ("Unknown key: %s (value: [%s])\n", keyname, gconf_value_get_string (value));
		}
	}

	g_free (serveruri_Key_path);        serveruri_Key_path       = NULL;
	g_free (username_Key_path);         username_Key_path        = NULL;
	g_free (policy_key_Key_path);       policy_key_Key_path      = NULL;
	g_free (calendar_folder_Key_path);  calendar_folder_Key_path = NULL;
	g_free (contact_folder_Key_path);   contact_folder_Key_path  = NULL;
	g_free (password_Key_path);         password_Key_path        = NULL;
	g_free (username_Key_path);         username_Key_path        = NULL;
	g_free (protover_Key_path);         protover_Key_path        = NULL;
	g_free (devover_Key_path);          devover_Key_path         = NULL;
	g_free (servoprotovers_Key_path);   servoprotovers_Key_path  = NULL;

	/* g_debug("eas_account_list_set_account_info--"); */
}

#if 0
void dump_accounts(EasAccountList *account_list)
{
	EIterator *iter = NULL;
	EasAccount *account = NULL;
	gint  num_of_accounts = 0;
	gint i=0;

	num_of_accounts = e_list_length((EList*)account_list);		
	g_debug(" There are %d accounts in GConf \n", num_of_accounts );
	
	if (!num_of_accounts)
		return;

	for (iter = e_list_get_iterator (E_LIST ( account_list) );
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = EAS_ACCOUNT (e_iterator_get (iter));

		g_debug("Account %d Info: \n", ++i);
		g_debug("account->uid=%s\n", eas_account_get_uid(account));
		g_debug("account->uri=%s\n", eas_account_get_uri(account));
		g_debug("account->username=%s\n", eas_account_get_username(account));
		g_debug("account->protocol_version=%d\n", eas_account_get_protocol_version(account));
		g_debug("account->policy_key=%s\n", eas_account_get_policy_key(account));
		g_debug("account->calendar_folder=%s\n", eas_account_get_calendar_folder(account));
		g_debug("account->contact_folder=%s\n", eas_account_get_contact_folder(account));
		g_debug("account->password=%s\n", eas_account_get_password(account));
	}

	g_object_unref (iter);
}
#endif

static void
gconf_accounts_changed (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	GSList *list = NULL, *l= NULL, *new_accounts = NULL;
	EasAccount *account = NULL;
	EList *old_accounts = NULL;
	EIterator *iter = NULL;
	gchar *uid = NULL;
	GSList* gconf_entry_list = NULL;
	GSList *account_uids_list = NULL;
	EasAccountInfo* acc_info = NULL; 
	EasAccountList *account_list = NULL;

	account_list = user_data;
	
	g_debug("gconf_accounts_changed++");
	old_accounts = e_list_duplicate (E_LIST (account_list));

	/*	
	Get list of children under /apps/activesyncd/accounts
	these should be the account uids. Loop through these uids and populate
	the account list.
	*/
	account_uids_list = gconf_client_all_dirs (client, EAS_ACCOUNT_ROOT, NULL);

	for (l = account_uids_list; l; l = l->next) 
	{
		uid = l->data;
		if (!uid)
			continue;
		/*
		 Get the key/value for an account with a given uid from GConf,
		 save it in EasAccountInfo and append it to the "list" object
		*/
		acc_info = g_new0 (EasAccountInfo, 1);
		gconf_entry_list = gconf_client_all_entries(client, uid, NULL);

		eas_account_list_set_account_info(acc_info, uid, gconf_entry_list);

#if 0
		g_debug ("uid              = %s", acc_info->uid); 
		g_debug ("serverUri        = %s", acc_info->serverUri);
		g_debug ("username         = %s", acc_info->username);
		g_debug ("policy_key       = %s", acc_info->policy_key);
		g_debug ("protocol_version = %d", acc_info->protocol_version);
		g_debug ("calendar_folder  = %s", acc_info->calendar_folder);
		g_debug ("contact_folder   = %s", acc_info->contact_folder);
		g_debug ("password         = %s", acc_info->password);
#endif

		list = g_slist_append (list, acc_info);

		// free gconf_entry_list
		if (gconf_entry_list) 
		{
			g_slist_foreach (gconf_entry_list, (GFunc) gconf_entry_free, NULL);
			g_slist_free (gconf_entry_list);
			gconf_entry_list=NULL;
		}
	}


	if (account_uids_list) {
		g_slist_foreach (account_uids_list, (GFunc) g_free, NULL);
		g_slist_free (account_uids_list);
		account_uids_list = NULL;
	}

	/* Begin processing changed, new or deleted accounts */
	for (l = list; l; l = l->next) 
	{
		uid = ((EasAccountInfo*)l->data)->uid;
		if (!uid)
			continue;

		/* See if this is an existing account */
		for (iter = e_list_get_iterator (old_accounts); e_iterator_is_valid (iter); e_iterator_next (iter)) 
		{
			account = (EasAccount *)e_iterator_get (iter);
			if (!strcmp (eas_account_get_uid(account), uid)) {
				/* The account still exists, so remove
				 * it from "old_accounts" and update it.
				 */
				e_iterator_delete (iter);
				if (eas_account_set_from_info (account, ((EasAccountInfo*)l->data))){
					g_signal_emit (account_list, signals[ACCOUNT_CHANGED], 0, account);
					/*g_debug("Account Changed: %s", eas_account_get_uid(account));*/
				}
				goto next;
			}
		}

		/* Must be a new account */
		account = eas_account_new_from_info (l->data);
		e_list_append (E_LIST (account_list), account);
		new_accounts = g_slist_prepend (new_accounts, account);

	next:
		g_object_unref (iter);
	}

	if (list) 
	{
		GSList *item = list;
		// Delete all the data from the Account Info's held in the list
		for (; item; item = item->next)
		{
	        EasAccountInfo *accountInfo = item->data;

	        g_free (accountInfo->uid); 
	        g_free (accountInfo->serverUri);
	        g_free (accountInfo->username);
	        g_free (accountInfo->policy_key);
	        g_free (accountInfo->contact_folder);
	        g_free (accountInfo->calendar_folder);
	        g_free (accountInfo->device_id);
	        g_free (accountInfo->password);
			g_free (accountInfo->server_protocols);
		}

		// Now free the memory allocated via g_new0
		g_slist_foreach (list, (GFunc) g_free, NULL);

		// Finally free the memory for the list itself
		g_slist_free (list);
	}

	/* Now emit signals for each added account */
	for (l = new_accounts; l; l = l->next) {
		account = l->data;
		g_signal_emit (account_list, signals[ACCOUNT_ADDED], 0, account);
		/*g_debug("Account Added: %s", eas_account_get_uid(account));*/
		g_object_unref (account);
	}
	g_slist_free (new_accounts);

	/* Anything left in old_accounts must have been deleted */
	for (iter = e_list_get_iterator (old_accounts);
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = (EasAccount *)e_iterator_get (iter);
		e_list_remove (E_LIST (account_list), account);
		g_signal_emit (account_list, signals[ACCOUNT_REMOVED], 0, account);
		/*g_debug("Account Deleted: %s", eas_account_get_uid(account));*/
	}

	/* dump_accounts(account_list); */
	
	g_object_unref (iter);
	g_object_unref (old_accounts);
	g_debug("gconf_accounts_changed--");
}

static gpointer
copy_func (gconstpointer data, gpointer closure)
{
	GObject *object = (GObject *)data;

	g_object_ref (object);
	return object;
}

static void
free_func (gpointer data, gpointer closure)
{
	g_object_unref (data);
}

/**
 * eas_account_list_new:
 * @client: a #GConfClient
 *
 * Reads the list of accounts from @client and listens for changes.
 * Will emit %account_added, %account_changed, and %account_removed
 * signals according to notifications from GConf.
 *
 * You can modify the list using e_list_append(), e_list_remove(), and
 * e_iterator_delete(). After adding, removing, or changing accounts,
 * you must call eas_account_list_save() to push the changes back to
 * GConf.
 *
 * Returns: the list of accounts
 **/
EasAccountList *
eas_account_list_new (GConfClient *gconf)
{
	EasAccountList *account_list;
	g_debug("eas_account_list_new++");
	g_return_val_if_fail (GCONF_IS_CLIENT (gconf), NULL);

	account_list = g_object_new (EAS_TYPE_ACCOUNT_LIST, NULL);
	eas_account_list_construct (account_list, gconf);

	g_debug("eas_account_list_new--");
	return account_list;
}

void
eas_account_list_construct (EasAccountList *account_list, GConfClient *gconf)
{
	g_return_if_fail (GCONF_IS_CLIENT (gconf));

	g_debug("eas_account_list_construct++");
	
	e_list_construct (E_LIST (account_list), copy_func, free_func, NULL);
	account_list->priv->gconf = gconf;
	g_object_ref (gconf);

	gconf_client_add_dir (account_list->priv->gconf,
			     EAS_ACCOUNT_ROOT,
			      GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);
	
	account_list->priv->notify_id =
		gconf_client_notify_add (account_list->priv->gconf,
					 EAS_ACCOUNT_ROOT,
					 gconf_accounts_changed, account_list,
					 NULL, NULL);

	gconf_accounts_changed (account_list->priv->gconf,
				account_list->priv->notify_id,
				NULL, account_list);

	g_debug("eas_account_list_construct--");
}

void
eas_account_list_save_account(EasAccountList *account_list,
						EasAccount *account)
{
	gchar* uid =NULL;
	
	g_debug("eas_account_list_save_account++");

	uid = eas_account_get_uid(account);
	if(!uid){
		g_warning("account must have a uid");
		return;
	}

	if (eas_account_get_uri(account)){
		gchar* serveruri_Key_path = NULL;
		serveruri_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVERURI);
		
		gconf_client_set_string (account_list->priv->gconf,
								serveruri_Key_path,
								eas_account_get_uri(account),
								NULL);
		g_free(serveruri_Key_path);
		serveruri_Key_path = NULL;
	}

	if (eas_account_get_username(account)){
		gchar* username_Key_path = NULL;
		username_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_USERNAME);
		
		gconf_client_set_string (account_list->priv->gconf,
						username_Key_path,
						eas_account_get_username(account),
						NULL);
		g_free(username_Key_path);
		username_Key_path = NULL;
	}
	
	if (eas_account_get_policy_key(account)){
		gchar* policy_key_Key_path = NULL;
		policy_key_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_POLICY_KEY);
		gconf_client_set_string (account_list->priv->gconf,
								policy_key_Key_path,
								eas_account_get_policy_key(account),
									NULL);
		g_free(policy_key_Key_path);
		policy_key_Key_path = NULL;
	}
	if (eas_account_get_contact_folder(account)){
		gchar* contact_folder_Key_path = NULL;
		contact_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CONTACT_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								contact_folder_Key_path,
								eas_account_get_contact_folder(account),
									NULL);
		g_free(contact_folder_Key_path);
		contact_folder_Key_path = NULL;
	}
	if (eas_account_get_calendar_folder(account)){
		gchar* calendar_folder_Key_path = NULL;
		calendar_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CALENDAR_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								calendar_folder_Key_path,
								eas_account_get_calendar_folder(account),
									NULL);
		g_free(calendar_folder_Key_path);
		calendar_folder_Key_path = NULL;
	}

	if (eas_account_get_password(account)){
		gchar* password_Key_path = NULL;
		password_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PASSWORD);

		gconf_client_set_string (account_list->priv->gconf,
						password_Key_path,
						eas_account_get_password(account),
						NULL);
		g_free (password_Key_path);
		password_Key_path = NULL;
	}

	if (eas_account_get_protocol_version(account)){
		gchar* protover_Key_path = NULL;
		protover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PROTOCOL_VERSION);

		gconf_client_set_int (account_list->priv->gconf,
				      protover_Key_path,
				      eas_account_get_protocol_version(account),
				      NULL);
		g_free (protover_Key_path);
		protover_Key_path = NULL;
	}
	if (eas_account_get_device_id(account)){
		gchar* devover_Key_path = NULL;
		devover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_DEVICE_ID);

		gconf_client_set_string (account_list->priv->gconf,
					  devover_Key_path,
					  eas_account_get_device_id(account),
					  NULL);
		g_free (devover_Key_path);
		devover_Key_path = NULL;
	}

	g_debug("eas_account_list_save_account--");
}

static void
eas_account_list_save_account_from_info(EasAccountList *account_list,
						EasAccountInfo *acc_info)
{
	gchar* uid =NULL;

	g_debug("eas_account_list_save_account++");
	
	uid = acc_info->uid;

	if (acc_info->serverUri){
		gchar* serveruri_Key_path = NULL;
		serveruri_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVERURI);
		
		gconf_client_set_string (account_list->priv->gconf,
								serveruri_Key_path,
								acc_info->serverUri,
								NULL);
		
		g_free(serveruri_Key_path);
		serveruri_Key_path = NULL;		
	}

	if (acc_info->username){
		gchar* username_Key_path = NULL;
		username_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_USERNAME);
		
		gconf_client_set_string (account_list->priv->gconf,
						username_Key_path,
						acc_info->username,
						NULL);
		g_free(username_Key_path);
		username_Key_path = NULL;

	}

	if (acc_info->policy_key){
		gchar* policy_key_Key_path = NULL;
		policy_key_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_POLICY_KEY);
		gconf_client_set_string (account_list->priv->gconf,
								policy_key_Key_path,
								acc_info->policy_key,
									NULL);
		g_free(policy_key_Key_path);
		policy_key_Key_path = NULL;

	}

	if (acc_info->contact_folder){
		gchar* contact_folder_Key_path = NULL;
		contact_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CALENDAR_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								contact_folder_Key_path,
								acc_info->contact_folder,
									NULL);
		g_free(contact_folder_Key_path);
		contact_folder_Key_path = NULL;

	}

	if (acc_info->calendar_folder){
		gchar* calendar_folder_Key_path = NULL;
		calendar_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CONTACT_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								calendar_folder_Key_path,
								acc_info->calendar_folder,
									NULL);
		g_free(calendar_folder_Key_path);
		calendar_folder_Key_path = NULL;

	}

	if (acc_info->password){
		gchar* password_Key_path = NULL;
		password_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PASSWORD);

		gconf_client_set_string (account_list->priv->gconf,
						password_Key_path,
						acc_info->password,
						NULL);
		g_free (password_Key_path);
		password_Key_path = NULL;
	}

	if (acc_info->protocol_version){
		gchar* protover_Key_path = NULL;
		protover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PROTOCOL_VERSION);

		gconf_client_set_int (account_list->priv->gconf,
				      protover_Key_path,
				      acc_info->protocol_version,
				      NULL);
		g_free (protover_Key_path);
		protover_Key_path = NULL;
	}

	if (acc_info->device_id){
		gchar* devover_Key_path = NULL;
		devover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_DEVICE_ID);

		gconf_client_set_string (account_list->priv->gconf,
						devover_Key_path,
						acc_info->device_id,
						NULL);
		g_free (devover_Key_path);
		devover_Key_path = NULL;
	}
	
	
	g_debug("eas_account_list_save_account--");
}


/**
 * eas_account_list_save:
 * @account_list: an #EasAccountList
 *
 * Saves @account_list to GConf. Signals will be emitted for changes.
 **/

void
eas_account_list_save_list (EasAccountList *account_list)
{
	GSList *list = NULL, *l = NULL;
	EasAccount *account = NULL;
	EasAccountInfo* acc_info = NULL; 	
	EIterator *iter = NULL;
	
	g_debug("eas_account_list_save++");

	for (iter = e_list_get_iterator (E_LIST (account_list));
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) 
	{
		account = (EasAccount *)e_iterator_get (iter);
		acc_info = eas_account_get_account_info(account);
#if 0 
		acc_info->uid = strdup(eas_account_get_uid(account)); 
		acc_info->serverUri = strdup(eas_account_get_uri(account));
		acc_info->username = strdup(eas_account_get_username(account));
		acc_info->policy_key = strdup(eas_account_get_policy_key(account));
		acc_info->protocol_version = eas_account_get_protocol_version(account);
		acc_info->calendar_folder = strdup(eas_account_get_calendar_folder(account));
		acc_info->contact_folder = strdup(eas_account_get_contact_folder(account));
		acc_info->password = strdup(eas_account_get_password(account));		
#endif
		list = g_slist_append (list, acc_info);
	}
	g_object_unref (iter);

	for(l = list; l; l = l->next){
		eas_account_list_save_account_from_info(account_list, acc_info);
	}

	while (list) {
		g_free (list->data);
		list = g_slist_remove (list, list->data);
	}

	gconf_client_suggest_sync (account_list->priv->gconf, NULL);

	g_debug("eas_account_list_save--");
}

void
eas_account_list_save_item(EasAccountList *account_list,
						EasAccount *account, eas_account_item_t type)
{
	gchar* uid =NULL;

	g_debug("eas_account_list_save_item++");
	/* pre-condition: account must have a uid */
	g_return_if_fail (eas_account_get_uid(account) != NULL);

	uid = eas_account_get_uid(account);
	
	switch (type) {
	case EAS_ACCOUNT_SERVER_URI:
		{
		gchar* serveruri_Key_path = NULL;
		serveruri_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVERURI);
		gconf_client_set_string (account_list->priv->gconf,
								serveruri_Key_path,
								eas_account_get_uri(account),
								NULL);
		
		g_free(serveruri_Key_path);
		serveruri_Key_path = NULL;		

		}
		break;
	case EAS_ACCOUNT_USERNAME:
		{
		gchar* username_Key_path = NULL;
		username_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_USERNAME);
		gconf_client_set_string (account_list->priv->gconf,
								username_Key_path,
								eas_account_get_username(account),
								NULL);
		g_free(username_Key_path);
		username_Key_path = NULL;
		}
		break;
	case EAS_ACCOUNT_POLICY_KEY:
		{
		gchar* policy_key_Key_path = NULL;
		policy_key_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_POLICY_KEY);
		gconf_client_set_string (account_list->priv->gconf,
								policy_key_Key_path,
								eas_account_get_policy_key(account),
									NULL);
		g_free(policy_key_Key_path);
		policy_key_Key_path = NULL;

		}
		break;
	case EAS_ACCOUNT_CALENDAR_FOLDER:
		{
		gchar* calendar_folder_Key_path = NULL;
		calendar_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CALENDAR_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								calendar_folder_Key_path,
								eas_account_get_calendar_folder(account),
									NULL);
		g_free(calendar_folder_Key_path);
		calendar_folder_Key_path = NULL;

		}
		break;
	case EAS_ACCOUNT_CONTACT_FOLDER:
		{
		gchar* contact_folder_Key_path = NULL;
		contact_folder_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_CONTACT_FOLDER);
		gconf_client_set_string (account_list->priv->gconf,
								contact_folder_Key_path,
								eas_account_get_contact_folder(account),
									NULL);
		g_free(contact_folder_Key_path);
		contact_folder_Key_path = NULL;

		}
		break;
	case EAS_ACCOUNT_PASSWORD:
		{
		gchar* password_Key_path = NULL;
		password_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PASSWORD);
		gconf_client_set_string (account_list->priv->gconf,
								password_Key_path,
								eas_account_get_password(account),
								NULL);

		g_free (password_Key_path);
		password_Key_path = NULL;
		}
		break;
	case EAS_ACCOUNT_PROTOCOL_VERSION:
		{
		gchar* protover_Key_path = NULL;
		protover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_PROTOCOL_VERSION);
		gconf_client_set_int (account_list->priv->gconf,
				      protover_Key_path,
				      eas_account_get_protocol_version(account),
				      NULL);
		g_free(protover_Key_path);
		protover_Key_path = NULL;

		}
		break;
	case EAS_ACCOUNT_DEVICE_ID:
		{
		gchar* devover_Key_path = NULL;
		devover_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_DEVICE_ID);
		gconf_client_set_string (account_list->priv->gconf,
								devover_Key_path,
								eas_account_get_device_id(account),
								NULL);

		g_free (devover_Key_path);
		devover_Key_path = NULL;
		}
		break;
	case EAS_ACCOUNT_SERVER_PROTOCOLS:
		{
		gboolean ret;

		gchar* server_protocols_Key_path = NULL;
		g_debug("EAS_ACCOUNT_SERVER_PROTOCOLS");
		server_protocols_Key_path = get_key_absolute_path(uid, EAS_ACCOUNT_KEY_SERVER_PROTOCOLS);
		ret = gconf_client_set_list (account_list->priv->gconf,
								server_protocols_Key_path,
								GCONF_VALUE_INT,
								eas_account_get_server_protocols(account),
								NULL);
		if(!ret){
			g_warning("Failed to set server protocol list");
		}
				
		g_free (server_protocols_Key_path);
		server_protocols_Key_path = NULL;			
		}
		break;
	default:
		g_warning("GConf item Type ( %d ) is not supported", type);
		break;
	}

	g_debug("eas_account_list_save_item--");
}


/**
 * eas_account_list_add:
 * @account_list: an #EasAccountList
 * @account: an #EasAccount
 *
 * Adds @account to @account_list and emits the
 * #EasAccountList::account-added signal.
 **/
void
eas_account_list_add (EasAccountList *account_list,
                    EasAccount *account)
{
	g_debug("eas_account_list_add++");
	g_return_if_fail (EAS_IS_ACCOUNT_LIST (account_list));
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	e_list_append (E_LIST (account_list), account);
	g_signal_emit (account_list, signals[ACCOUNT_ADDED], 0, account);
	g_debug("eas_account_list_add--");
}

/**
 * eas_account_list_change:
 * @account_list: an #EasAccountList
 * @account: an #EasAccount
 *
 * Emits the #EasAccountList::account-changed signal.
 **/
void
eas_account_list_change (EasAccountList *account_list,
                       EasAccount *account)
{
	g_debug("eas_account_list_change++");
	
	g_return_if_fail (EAS_IS_ACCOUNT_LIST (account_list));
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	g_signal_emit (account_list, signals[ACCOUNT_CHANGED], 0, account);
	g_debug("eas_account_list_change--");
}

/**
 * eas_account_list_remove:
 * @account_list: an #EasAccountList
 * @account: an #EasAccount
 *
 * Removes @account from @account list, and emits the
 * #EasAccountList::account-removed signal. **/
void
eas_account_list_remove (EasAccountList *account_list,
                       EasAccount *account)
{
	g_debug("eas_account_list_remove++");
	g_return_if_fail (EAS_IS_ACCOUNT_LIST (account_list));
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	/* not sure if need to ref but no harm */
	g_object_ref (account);
	e_list_remove ((EList *) account_list, account);
	g_signal_emit (account_list, signals[ACCOUNT_REMOVED], 0, account);
	g_object_unref (account);
	g_debug("eas_account_list_remove--");	
}


/**
 * e_account_list_find:
 * @account_list: an #EasAccountList
 * @type: type of search
 * @key: the search key
 *
 * Perform a search of @account_list on a single key.
 *
 * @type must be set from one of the following search types.
 *
 * Returns: The account or %NULL if it doesn't exist.
 **/
EasAccount *
eas_account_list_find (EasAccountList *account_list,
                     eas_account_find_t type,
                     const gchar *key)
{

	EIterator *it;
	EasAccount *account = NULL;
	g_debug("eas_account_list_find++");

	if (!key)
		return NULL;

	for (it = e_list_get_iterator ((EList *)account_list);
	     e_iterator_is_valid (it);
	     e_iterator_next (it)) {
		gint found = 0;

		account = (EasAccount *)e_iterator_get (it);

		switch (type) {
		case EAS_ACCOUNT_FIND_ACCOUNT_UID:
			if (eas_account_get_uid(account))
				found = strcmp (eas_account_get_uid(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_SERVER_URI:
			if (eas_account_get_uri(account))
				found = strcmp (eas_account_get_uri(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_USER_NAME:
			if (eas_account_get_username(account))
				found = strcmp (eas_account_get_username(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_POLICY_KEY:
			if (eas_account_get_policy_key(account))
				found = strcmp (eas_account_get_policy_key(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_CALENDAR_FOLDER:
			if (eas_account_get_calendar_folder(account))
				found = strcmp (eas_account_get_calendar_folder(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_CONTACT_FOLDER:
			if (eas_account_get_contact_folder(account))
				found = strcmp (eas_account_get_contact_folder(account), key) == 0;
			break;
		case EAS_ACCOUNT_FIND_PASSWORD:
			if (eas_account_get_password(account))
				found = strcmp (eas_account_get_password(account), key) == 0;
			break;
				
		}

		if (found)
			break;

		account = NULL;
	}
	g_object_unref (it);

	if (account)
		g_object_ref (G_OBJECT(account));
	g_debug("eas_account_list_find--");
	return account;
}

