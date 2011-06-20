

#include <glib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "eas-account-list.h"
#include "eas-account.h"

#include <string.h>

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

static void
gconf_accounts_changed (GConfClient *client, guint cnxn_id,
			GConfEntry *entry, gpointer user_data)
{
	EasAccountList *account_list = user_data;
	GSList *list, *l, *new_accounts = NULL;
	EasAccount *account;
	EList *old_accounts;
	EIterator *iter;
	gchar *uid;
	g_debug("gconf_accounts_changed++");
	old_accounts = e_list_duplicate (E_LIST (account_list));

	list = gconf_client_get_list (client, "/apps/activesyncd/accounts",
				      GCONF_VALUE_STRING, NULL);
	for (l = list; l; l = l->next) {
		uid = eas_account_uid_from_xml (l->data);
		if (!uid)
			continue;

		/* See if this is an existing account */
		for (iter = e_list_get_iterator (old_accounts);
		     e_iterator_is_valid (iter);
		     e_iterator_next (iter)) {
			account = (EasAccount *)e_iterator_get (iter);
			if (!strcmp (account->uid, uid)) {
				/* The account still exists, so remove
				 * it from "old_accounts" and update it.
				 */
				e_iterator_delete (iter);
				if (eas_account_set_from_xml (account, l->data))
					g_signal_emit (account_list, signals[ACCOUNT_CHANGED], 0, account);
				goto next;
			}
		}

		/* Must be a new account */
		account = eas_account_new_from_xml (l->data);
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

	/* Now emit signals for each added account. (We do this after
	 * adding all of them because otherwise if the signal handler
	 * calls eas_account_list_get_default_account() it will end up
	 * causing the first account in the list to become the
	 * default.)
	 */
	for (l = new_accounts; l; l = l->next) {
		account = l->data;
		g_signal_emit (account_list, signals[ACCOUNT_ADDED], 0, account);
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
			     "/apps/activesyncd/accounts",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	account_list->priv->notify_id =
		gconf_client_notify_add (account_list->priv->gconf,
					 "/apps/activesyncd/accounts",
					 gconf_accounts_changed, account_list,
					 NULL, NULL);

	gconf_accounts_changed (account_list->priv->gconf,
				account_list->priv->notify_id,
				NULL, account_list);

	g_debug("eas_account_list_construct--");
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
	GSList *list = NULL;
	EasAccount *account;
	EIterator *iter;
	gchar *xmlbuf;
	g_debug("eas_account_list_save++");

	for (iter = e_list_get_iterator (E_LIST (account_list));
	     e_iterator_is_valid (iter);
	     e_iterator_next (iter)) {
		account = (EasAccount *)e_iterator_get (iter);

		xmlbuf = eas_account_to_xml (account);
		if (xmlbuf)
			list = g_slist_append (list, xmlbuf);
	}
	g_object_unref (iter);

	gconf_client_set_list (account_list->priv->gconf,
			       "/apps/activesyncd/accounts",
			       GCONF_VALUE_STRING, list, NULL);

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
 * #EasAccountList::account-removed signal.  If @account was the default
 * account, then the first account in @account_list becomes the new default.
 **/
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
 * @type must be set from one of the following search types:
 * E_ACCOUNT_FIND_NAME - Find an account by account name.
 * E_ACCOUNT_FIND_ID_NAME - Find an account by the owner's identity name.
 * E_ACCOUNT_FIND_ID_ADDRESS - Find an account by the owner's identity address.
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
			found = strcmp (account->uid, key) == 0;
			break;
		case EAS_ACCOUNT_FIND_SERVER_URI:
			found = strcmp (account->serverUri, key) == 0;
			break;
		case EAS_ACCOUNT_FIND_USER_NAME:
			if (account->username)
				found = strcmp (account->username, key) == 0;
			break;
		case EAS_ACCOUNT_FIND_PASSWORD:
			if (account->password)
				found = strcmp (account->username, key) == 0;
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

