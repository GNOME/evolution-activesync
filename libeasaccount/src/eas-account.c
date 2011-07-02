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

#include "eas-account.h"
#include <string.h>


enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void finalize (GObject *);

G_DEFINE_TYPE (EasAccount, eas_account, G_TYPE_OBJECT)

struct	_EasAccountPrivate
{
	 gchar* uid; 
	 gchar* serverUri;
	 gchar* username;
	 gchar* policy_key;
	 gchar* password;
};
	
#define EAS_ACCOUNT_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ACCOUNT, EasAccountPrivate))

static void
eas_account_class_init (EasAccountClass *account_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (account_class);

	/* virtual method override */
	object_class->finalize = finalize;

	g_type_class_add_private (account_class, sizeof (EasAccountPrivate));
	
	signals[CHANGED] =
		g_signal_new("changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET (EasAccountClass, changed),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__INT,
			     G_TYPE_NONE, 1,
			     G_TYPE_INT);
}

static void
eas_account_init (EasAccount *account)
{
	EasAccountPrivate *priv = NULL;
	g_debug("eas_account_init++");
	account->priv = priv = EAS_ACCOUNT_PRIVATE(account);

	priv->uid = NULL;
	priv->serverUri = NULL;
	priv->username = NULL;
	priv->policy_key = NULL;
	priv->password = NULL;
	g_debug("eas_account_init--");
}


static void
finalize (GObject *object)
{
	g_debug("finalize++");	
	EasAccount *account = EAS_ACCOUNT (object);
	EasAccountPrivate *priv =account->priv;

	g_free (priv->uid);
	g_free (priv->serverUri);
	g_free (priv->username);
	g_free (priv->policy_key);
	g_free (priv->password);

	G_OBJECT_CLASS (eas_account_parent_class)->finalize (object);
	g_debug("finalize--");		
}

/**
 * eas_account_new:
 *
 * Returns: a blank new account which can be filled in and
 * added to an #EasAccountList.
 **/
EasAccount *
eas_account_new (void)
{
	EasAccount *account;
	g_debug("eas_account_new++");
	account = g_object_new (EAS_TYPE_ACCOUNT, NULL);
	g_debug("eas_account_new--");
	return account;
}

EasAccount *
eas_account_new_from_info (EasAccountInfo* accountinfo)
{
	EasAccount *account = NULL;
	g_debug("eas_account_new_from_info++");
	account = g_object_new (EAS_TYPE_ACCOUNT, NULL);

	if(!eas_account_set_from_info(account, accountinfo)){
		g_object_unref (account);
		return NULL;
	}

	g_debug("eas_account_new_from_info--");
	return account;
}

void
eas_account_set_uid (EasAccount *account, const gchar* uid)
{
	/* g_debug("eas_account_set_uid++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(account->priv->uid == NULL){
		if(uid != NULL){
			account->priv->uid = g_strdup (uid);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "uid changed: [%s]\n", account->priv->uid);
		}
	}else{
		if(uid != NULL){
			if(strcmp(account->priv->uid, uid) != 0){
				g_free(account->priv->uid);
				account->priv->uid = g_strdup (uid);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "uid changed: [%s]\n", account->priv->uid);
				}		
		}else{
				g_free(account->priv->uid);
				account->priv->uid = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "uid changed: [%s]\n", account->priv->uid);		
		}
	}

	/* g_debug("eas_account_set_uid--"); */
}

void
eas_account_set_uri (EasAccount *account, const gchar* uri)
{
	/* g_debug("eas_account_set_uri++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(account->priv->serverUri == NULL){
		if(uri != NULL){
			account->priv->serverUri = g_strdup (uri);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "uri changed: [%s]\n", account->priv->serverUri);	
		}
	}else{
		if(uri != NULL){
			if(strcmp(account->priv->serverUri, uri) != 0){
				g_free(account->priv->serverUri);
				account->priv->serverUri = g_strdup (uri);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "uri changed: [%s]\n", account->priv->serverUri);
				}		
		}else{
				g_free(account->priv->serverUri);
				account->priv->serverUri = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "uri changed: [%s]\n", account->priv->serverUri);		
		}
	}
	
	/* g_debug("eas_account_set_uri--"); */
}

void
eas_account_set_username (EasAccount *account, const gchar* username)
{
	/* g_debug("eas_account_set_username++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));	

	if(account->priv->username == NULL){
		if(username != NULL){
			account->priv->username = g_strdup (username);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "username changed: [%s]\n", account->priv->username);	
		}
	}else{
		if(username != NULL){
			if(strcmp(account->priv->username, username) != 0){
				g_free(account->priv->username);
				account->priv->username = g_strdup (username);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "username changed: [%s]\n", account->priv->username);
				}
		}else{
				g_free(account->priv->username);
				account->priv->username = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "username changed: [%s]\n", account->priv->username);
		}
	}
	
	/* g_debug("eas_account_set_username--"); */
}

void
eas_account_set_policy_key (EasAccount *account, const gchar* policy_key)
{
	/* g_debug("eas_account_set_policy_key++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(account->priv->policy_key == NULL){
		if(policy_key != NULL){
			account->priv->policy_key = g_strdup (policy_key);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "policy_key changed: [%s]\n", account->priv->policy_key);
		}
	}else{
		if(policy_key != NULL){
			if(strcmp(account->priv->policy_key, policy_key) != 0){
				g_free(account->priv->policy_key);
				account->priv->policy_key = g_strdup (policy_key);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "policy_key changed: [%s]\n", account->priv->policy_key);
				}
		}else{
				g_free(account->priv->policy_key);
				account->priv->policy_key = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "policy_key changed: [%s]\n", account->priv->policy_key);
		}
	}

	/* g_debug("eas_account_set_policy_key--"); */
}

void
eas_account_set_password (EasAccount *account, const gchar* password)

{
	/*g_debug("eas_account_set_password++");*/
	g_return_if_fail (EAS_IS_ACCOUNT (account));
	if(account->priv->password == NULL){
		if(password != NULL){
			account->priv->password = g_strdup (password);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "password changed: [%s]\n", account->priv->password);
		}
	}else{
		if(password != NULL){
			if(strcmp(account->priv->password, password) != 0){
				g_free(account->priv->password);
				account->priv->password = g_strdup (password);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "password changed: [%s]\n", account->priv->password);
			}
		}else{
				g_free(account->priv->password);
				account->priv->password = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "password changed: [%s]\n", account->priv->password);
		}
	} 
	/*g_debug("eas_account_set_password--");*/
}

gchar*
eas_account_get_uid (const EasAccount *account)
{
	return account->priv->uid;
}

gchar* 
eas_account_get_uri (const EasAccount *account)
{
	return account->priv->serverUri;
}

gchar* 
eas_account_get_username (const EasAccount *account)
{
	return account->priv->username;
}

gchar*
eas_account_get_policy_key (const EasAccount *account)
{
	return account->priv->policy_key;
}

gchar*
eas_account_get_password (const EasAccount *account)
{
	return account->priv->password;
}

gboolean
eas_account_set_from_info(EasAccount *account, const EasAccountInfo* accountinfo)
{
	/*g_debug("eas_account_set_from_info++"); */
	/* account must have a uid*/
	if (!accountinfo->uid)
		return FALSE;

	eas_account_set_uid (account, accountinfo->uid);
	eas_account_set_uri (account, accountinfo->serverUri);
	eas_account_set_username (account, accountinfo->username);
	eas_account_set_policy_key (account, accountinfo->policy_key);
	eas_account_set_password (account, accountinfo->password);
	/* g_debug("eas_account_set_from_info--");	*/
	return TRUE;
}

/*
	EasAccountInfo not owned
 */
EasAccountInfo*
eas_account_get_account_info(const EasAccount *account)
{
	EasAccountInfo* acc_info = NULL;

	acc_info = g_new0 (EasAccountInfo, 1);
	if (acc_info){
		acc_info->uid = account->priv->uid;
		acc_info->serverUri = account->priv->serverUri;
		acc_info->username = account->priv->username;
		acc_info->policy_key = account->priv->policy_key;
		acc_info->password = account->priv->password;		
	}
	return acc_info;
}
