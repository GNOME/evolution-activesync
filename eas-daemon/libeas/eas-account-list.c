

#include <glib.h>


#include "eas-account-list.h"
#include "eas-account.h"

#include <string.h>


#define EAS_ACCOUNT_ROOT			"/apps/activesyncd/accounts"
#define EAS_ACCOUNT_KEY_SERVERURI	"/serverUri"
#define EAS_ACCOUNT_KEY_USERNAME	"/username"
#define EAS_ACCOUNT_KEY_PASSWORD	"/password"


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

static void dispose (GObject *);
static void finalize (GObject *);

G_DEFINE_TYPE (EasAccountList, eas_account_list, E_TYPE_LIST)

static void
eas_account_list_class_init (EasAccountListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	g_debug("eas_account_list_class_init++");
	/* virtual method override */
	object_class->dispose = dispose;
	object_class->finalize = finalize;

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
dispose (GObject *object)
{
	EasAccountList *account_list = EAS_ACCOUNT_LIST (object);
	g_debug("dispose++");
	if (account_list->priv->gconf) {
		if (account_list->priv->notify_id) {
			gconf_client_notify_remove (account_list->priv->gconf,
						    account_list->priv->notify_id);
		}
		g_object_unref (account_list->priv->gconf);
		account_list->priv->gconf = NULL;
	}
	g_debug("dispose--");
	G_OBJECT_CLASS (eas_account_list_parent_class)->dispose (object);
}

static void
finalize (GObject *object)
{
	EasAccountList *account_list = EAS_ACCOUNT_LIST (object);
	g_debug("finalize++");
	g_free (account_list->priv);
	g_debug("finalize--");
	G_OBJECT_CLASS (eas_account_list_parent_class)->finalize (object);
}

void
eas_account_list_set_account_info(EasAccountInfo *acc, const gchar* uid_path, GConfEntry* entry)
{
	const GConfValue* value = NULL;
	const gchar* keyname = NULL;
	gchar* strValue = NULL;
	gchar* uid = NULL;
	g_debug("eas_account_list_set_account_info++");
	g_return_if_fail (acc != NULL);
	g_return_if_fail (uid_path != NULL);
	g_return_if_fail (entry != NULL);
	
	keyname = gconf_entry_get_key(entry);
	if (keyname == NULL) {
		g_debug("Couldn't get the key name - this could be a delete notification!\n");
		return;
	}

	value = gconf_entry_get_value(entry);
	if (value == NULL) {
		g_debug("Couldn't get the key value - this could be a delete notification!\n");
		return;
	}

	
	/* strip the EAS_ACCOUNT_ROOT from the uid_path to get the uid only */
	gint last_token = 4;
	gchar **str_array = NULL;
	str_array = g_strsplit(uid_path, "/", -1);
	uid = g_strdup(str_array[last_token]);
	g_debug("uid=%s\n", uid);
	/* free the vector */
	g_strfreev(str_array);

	
	strValue = gconf_value_to_string(value); /* need to be freed */

	/* Concatenate "ROOT + UID + KEY" */
	gulong string_Key_len;
	string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) + strlen(EAS_ACCOUNT_KEY_SERVERURI) + 1;
	gchar serveruri_Key_path[string_Key_len];
	g_snprintf(serveruri_Key_path, string_Key_len, "%s/%s%s", EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_SERVERURI);
	
	string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) + strlen(EAS_ACCOUNT_KEY_USERNAME) + 1;
	gchar username_Key_path[string_Key_len];
	g_snprintf(username_Key_path, string_Key_len, "%s/%s%s", EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_USERNAME);
	
	string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) + strlen(EAS_ACCOUNT_KEY_PASSWORD)+ 1;
	gchar password_Key_path[string_Key_len];
	g_snprintf(password_Key_path, string_Key_len, "%s/%s%s", EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_PASSWORD);

	acc->uid = g_strdup(uid);
	if (strcmp(keyname, serveruri_Key_path) == 0) {
		acc->serverUri = g_strdup(strValue);
		g_debug( "serverUri changed: [%s]\n", strValue);
	} else if (strcmp(keyname, username_Key_path) == 0) {
		acc->username = g_strdup(strValue);
		g_debug( "username changed: [%s]\n", strValue);
	} else if (strcmp(keyname, password_Key_path) == 0) {
		acc->password = g_strdup(strValue);
		g_debug( "password  changed: [%s]\n", strValue);
	}  else {
		g_debug( "Unknown key: %s (value: [%s])\n", keyname, strValue);
	}

	g_free (uid);
	/* Free the string representation of the value. */
	g_free(strValue);

	g_debug("eas_account_list_set_account_info--");
}

static void
gconf_accounts_changed (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	EasAccountList *account_list = user_data;
	GSList *list =NULL, *l=NULL, *ll = NULL, *new_accounts = NULL;
	EasAccount *account = NULL;
	EList *old_accounts = NULL;;
	EIterator *iter = NULL;;
	gchar *uid = NULL;;	
	GSList* gconf_entry_list = NULL;
	GSList *account_uids_list = NULL;	
	EasAccountInfo* acc = NULL; 

	g_debug("gconf_accounts_changed++");
	old_accounts = e_list_duplicate (E_LIST (account_list));

	/*	
	Get list of children under /apps/activesyncd/accounts
	these should be the account uids. Loop through these uids and populate
	the account list.
	*/
	account_uids_list = gconf_client_all_dirs (client, EAS_ACCOUNT_ROOT, NULL);

	for (l = account_uids_list; l; l = l->next) {
		uid = l->data;
		if (!uid)
			continue;
	/*
	 Get the key/value for an account with given uid from GConf
	 save it in EasAccountInfo	and append it to the "list" object
	*/
	acc = g_new0 (EasAccountInfo, 1);
	gconf_entry_list = gconf_client_all_entries(client, uid, NULL);
	for (ll = gconf_entry_list; ll; ll = ll->next) {
		eas_account_list_set_account_info(acc, uid, ll->data );
	}

	g_debug("uid =%s", acc->uid); 
	g_debug("serverUri =%s", acc->serverUri);
	g_debug("username =%s", acc->username);
	g_debug("password =%s", acc->password);

	list = g_slist_append (list, acc);

	// free gconf_entry_list
	if (gconf_entry_list) {
		g_slist_foreach (gconf_entry_list, (GFunc) gconf_entry_free, NULL);
		g_slist_free (gconf_entry_list);
		gconf_entry_list=NULL;
	}
	
	}


	//free account_uids_list 
	if (account_uids_list) {
		g_slist_foreach (account_uids_list, (GFunc) g_free, NULL);
		g_slist_free (account_uids_list);
		account_uids_list = NULL;
	}
	

	/* Begin 	*/
	for (l = list; l; l = l->next) {
		uid = ((EasAccountInfo*)l->data)->uid;
		if (!uid)
			continue;
		
		/* See if this is an existing account */
		for (iter = e_list_get_iterator (old_accounts);
		     e_iterator_is_valid (iter);
		     e_iterator_next (iter)) {
			account = (EasAccount *)e_iterator_get (iter);
			if (!strcmp (eas_account_get_uid(account), uid)) {
				/* The account still exists, so remove
				 * it from "old_accounts" and update it.
				 */
				e_iterator_delete (iter);
				/*if (eas_account_set_from_xml (account, l->data)) */
				if (eas_account_set_from_info (account, ((EasAccountInfo*)l->data))){
					g_signal_emit (account_list, signals[ACCOUNT_CHANGED], 0, account);
					g_debug("Account Changed: %s", eas_account_get_uid(account));
				}
				goto next;
			}
		}

		/* Must be a new account */
		account = eas_account_new_from_info (l->data);
		e_list_append (E_LIST (account_list), account);
		new_accounts = g_slist_prepend (new_accounts, account);

	next:
		g_free (uid);
		g_object_unref (iter);
	}

	

	if (list) {
		g_slist_foreach (list, (GFunc) g_free, NULL);
		g_slist_free (list);
	}

	/* Now emit signals for each added account */
	for (l = new_accounts; l; l = l->next) {
		account = l->data;
		g_signal_emit (account_list, signals[ACCOUNT_ADDED], 0, account);
		g_debug("Account Added: %s", eas_account_get_uid(account));
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
		g_debug("Account Deleted: %s", eas_account_get_uid(account));
	}
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
eas_account_gconf_save(GConfClient *client, const GSList *list)
{
	g_debug("eas_account_gconf_save++");
	GSList* l = NULL;
	gchar* uid = NULL;

	
	g_debug("eas_account_gconf_save++ 01");

	for(l = list; l; l = l->next){
		g_debug("eas_account_gconf_save++ 01 ***");
		uid = eas_account_get_uid((EasAccount*)l->data);
		if (!uid)
			continue;
//TODO:?
//		uid = strdup(eas_account_get_uid((EasAccount*)l->data));

		/* Concatenate to form absolute paths for keys */
		/* e.g. serveruri_Key_path = EAS_ACCOUNT_ROOT + "/" + uid +
		   EAS_ACCOUNT_KEY_SERVERURI*/
		gulong string_Key_len;
		string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) +
								strlen(EAS_ACCOUNT_KEY_SERVERURI) + 1;
	g_debug("eas_account_gconf_save++ 01-A");
		gchar serveruri_Key_path[string_Key_len];
	g_debug("eas_account_gconf_save++ 01-A");		
		g_snprintf(serveruri_Key_path, string_Key_len, "%s/%s%s",
					EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_SERVERURI);
	g_debug("eas_account_gconf_save++ 01-A");
		string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) +
								strlen(EAS_ACCOUNT_KEY_USERNAME) + 1;
		gchar username_Key_path[string_Key_len];
		g_snprintf(username_Key_path, string_Key_len, "%s/%s%s",
					EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_USERNAME);

		string_Key_len = strlen(EAS_ACCOUNT_ROOT) + 1 + strlen(uid) +
								strlen(EAS_ACCOUNT_KEY_PASSWORD)+ 1;
		gchar password_Key_path[string_Key_len];
		g_snprintf(password_Key_path, string_Key_len, "%s/%s%s",
					EAS_ACCOUNT_ROOT, uid, EAS_ACCOUNT_KEY_PASSWORD);	
	g_debug("eas_account_gconf_save++ 02");
		gconf_client_set_string (client,
								serveruri_Key_path,
								eas_account_get_uri((EasAccount*)l->data),
								NULL);

		gconf_client_set_string (client,
								username_Key_path,
								eas_account_get_username((EasAccount*)l->data),
								NULL);

		gconf_client_set_string (client,
								password_Key_path,
								eas_account_get_password((EasAccount*)l->data),
								NULL);
	g_debug("eas_account_gconf_save++ 03");
//END TODO:
//		g_free (uid);
	}
	g_debug("eas_account_gconf_save--");
}
/**
 * eas_account_list_save:
 * @account_list: an #EasAccountList
 *
 * Saves @account_list to GConf. Signals will be emitted for changes.
 **/

void
eas_account_list_save (EasAccountList *account_list)
{
	g_debug("eas_account_list_save++");	

	GSList *list = NULL;
	EasAccount *account =NULL;
	EIterator *iter =NULL;

	for (iter = e_list_get_iterator (E_LIST (account_list));
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = (EasAccount *)e_iterator_get (iter);
		list = g_slist_append (list, account);
	}
	g_object_unref (iter);

	eas_account_gconf_save(account_list->priv->gconf, list);
	
	while (list) {
		g_free (list->data);
		list = g_slist_remove (list, list->data);
	}

	gconf_client_suggest_sync (account_list->priv->gconf, NULL);

	g_debug("eas_account_list_save--");
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
const EasAccount *
eas_account_list_find (EasAccountList *account_list,
                     eas_account_find_t type,
                     const gchar *key)
{

	EIterator *it;
	const EasAccount *account = NULL;
	g_debug("eas_account_list_find++");
	/* this could use a callback for more flexibility ...
	   ... but this makes the common cases easier */

	if (!key)
		return NULL;

	for (it = e_list_get_iterator ((EList *)account_list);
	     e_iterator_is_valid (it);
	     e_iterator_next (it)) {
		gint found = 0;

		account = (const EasAccount *)e_iterator_get (it);

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

	g_debug("eas_account_list_find--");
	return account;
}

