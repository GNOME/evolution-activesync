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

static void eas_account_finalize (GObject *);

G_DEFINE_TYPE (EasAccount, eas_account, G_TYPE_OBJECT)

struct	_EasAccountPrivate
{
	 gchar* uid; 
	 gchar* serverUri;
	 gchar* username;
	 gchar* policy_key;
	 gchar* calendar_folder;
	 gchar* contact_folder;
	 gchar* password;
	 gchar* device_id;
     int protocol_version;	
	 GSList* server_protocols;	// list of protocols supported by server, eg 120, 121, 140, 141
};
	
#define EAS_ACCOUNT_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ACCOUNT, EasAccountPrivate))

static void
eas_account_class_init (EasAccountClass *account_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (account_class);

	/* virtual method override */
	object_class->finalize = eas_account_finalize;

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
	priv->calendar_folder = NULL;
	priv->contact_folder = NULL;
	priv->password = NULL;
	priv->device_id = NULL;
	priv->server_protocols = NULL;
	g_debug("eas_account_init--");
}


static void
eas_account_finalize (GObject *object)
{	
	EasAccount *account = EAS_ACCOUNT (object);
	EasAccountPrivate *priv = account->priv;
	g_debug("eas_account_finalize++");
	g_free (priv->uid);
	g_free (priv->serverUri);
	g_free (priv->username);
	g_free (priv->policy_key);
	g_free (priv->calendar_folder);
	g_free (priv->contact_folder);
	g_free (priv->password);
	g_free (priv->device_id);
	g_slist_free(priv->server_protocols);
	G_OBJECT_CLASS (eas_account_parent_class)->finalize (object);
	g_debug("eas_account_finalize--");
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
eas_account_set_calendar_folder (EasAccount *account, const gchar* calendar_folder)
{
	/* g_debug("eas_account_set_calendar_folder++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(account->priv->calendar_folder == NULL){
		if(calendar_folder != NULL){
			account->priv->calendar_folder = g_strdup (calendar_folder);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "calendar_folder changed: [%s]\n", account->priv->calendar_folder);
		}
	}else{
		if(calendar_folder != NULL){
			if(strcmp(account->priv->calendar_folder, calendar_folder) != 0){
				g_free(account->priv->calendar_folder);
				account->priv->calendar_folder = g_strdup (calendar_folder);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "calendar_folder changed: [%s]\n", account->priv->calendar_folder);
				}
		}else{
				g_free(account->priv->calendar_folder);
				account->priv->calendar_folder = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "calendar_folder changed: [%s]\n", account->priv->calendar_folder);
		}
	}

	/* g_debug("eas_account_set_calendar_folder--"); */
}
void
eas_account_set_contact_folder (EasAccount *account, const gchar* contact_folder)
{
	/* g_debug("eas_account_set_contact_folder++"); */
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(account->priv->contact_folder == NULL){
		if(contact_folder != NULL){
			account->priv->contact_folder = g_strdup (contact_folder);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "contact_folder changed: [%s]\n", account->priv->contact_folder);
		}
	}else{
		if(contact_folder != NULL){
			if(strcmp(account->priv->contact_folder, contact_folder) != 0){
				g_free(account->priv->contact_folder);
				account->priv->contact_folder = g_strdup (contact_folder);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "contact_folder changed: [%s]\n", account->priv->contact_folder);
				}
		}else{
				g_free(account->priv->contact_folder);
				account->priv->contact_folder = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "contact_folder changed: [%s]\n", account->priv->contact_folder);
		}
	}

	/* g_debug("eas_account_set_contact_folder--"); */
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

void
eas_account_set_device_id (EasAccount *account, const gchar* device_id)

{
	g_debug("eas_account_set_device_id++");
	g_return_if_fail (EAS_IS_ACCOUNT (account));
	if(account->priv->device_id == NULL){
		if(device_id != NULL){
			account->priv->device_id = g_strdup (device_id);
			g_signal_emit (account, signals[CHANGED], 0, -1);
			g_debug( "device_id changed: [%s]\n", account->priv->device_id);
		}
	}else{
		if(device_id != NULL){
			if(strcmp(account->priv->device_id, device_id) != 0){
				g_free(account->priv->device_id);
				account->priv->device_id = g_strdup (device_id);
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "device_id changed: [%s]\n", account->priv->device_id);
			}
		}else{
				g_free(account->priv->device_id);
				account->priv->device_id = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "device_id changed: [%s]\n", account->priv->device_id);
		}
	} 
	g_debug("eas_account_set_device_id--");
}

void
eas_account_set_protocol_version (EasAccount *account, int protocol_version)
{
	/*g_debug("eas_account_set_protocol_version++");*/
	g_return_if_fail (EAS_IS_ACCOUNT (account));
	if(account->priv->protocol_version != protocol_version){
		account->priv->protocol_version = protocol_version;
		g_signal_emit (account, signals[CHANGED], 0, -1);
		g_debug( "protocol_version changed: [%d]\n", account->priv->protocol_version);
	}
	/*g_debug("eas_account_set_protocol_version--");*/
}

/* 
 * Takes a list of protocols numbers (eg 120, 121, 140..) 
 * and saves it in the account
 */
void
eas_account_set_server_protocols (EasAccount *account, const GSList *server_protocols)
{
	EasAccountPrivate *priv = account->priv;
	guint len = 0;
	guint i, j;
	GSList* server_protocols_new = (GSList *)server_protocols;	// avoid const warnings
	
	g_debug("eas_account_set_server_protocols++");
	g_return_if_fail (EAS_IS_ACCOUNT (account));

	if(server_protocols_new != NULL){
		len = g_slist_length (server_protocols_new);
	}
	
	if(priv->server_protocols == NULL){	// set previously unset:
		for(i = 0; i < len; i++)
		{
			priv->server_protocols = g_slist_append(priv->server_protocols, GUINT_TO_POINTER (g_slist_nth_data(server_protocols_new, i)));
		}
		g_signal_emit (account, signals[CHANGED], 0, -1);
		g_debug( "server_protocols changed:");
		for(i = 0; i < g_slist_length(priv->server_protocols); i++)
		{
			g_debug( "[%p]", g_slist_nth_data (priv->server_protocols, i)); 
		}
	}else{	
		if(len){ // compare to current, if different update:
			gboolean changed = FALSE;
			if(len == g_slist_length (priv->server_protocols)){// same length, may be unchanged
				for (i = 0; i < len; i++){
					guint proto_curr = GPOINTER_TO_UINT (g_slist_nth_data (server_protocols_new, i));
					gboolean found = FALSE;
					for(j = 0; j < len; j++){
						guint proto_new = GPOINTER_TO_UINT (g_slist_nth_data (priv->server_protocols,j));
						if( proto_curr == proto_new){
							// got a match
							found = TRUE;
							break;
						}
					}
					if(!found){
						g_debug("changed");
						changed = TRUE;
						break;
					}	
				}	
			}else{
				changed = TRUE;
			}
			
			if(changed){ 
				g_slist_free(priv->server_protocols);
				for(i = 0; i < len; i++)
				{			
					guint proto = GPOINTER_TO_UINT (g_slist_nth_data (server_protocols_new, i));
					priv->server_protocols = g_slist_append(priv->server_protocols, GUINT_TO_POINTER (proto));
				}				
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "server_protocols changed: NULL\n");
			}else{
				g_debug("unchanged");
			}
		}else{	// delete:
				g_slist_free(priv->server_protocols);			
				account->priv->server_protocols = NULL;
				g_signal_emit (account, signals[CHANGED], 0, -1);
				g_debug( "server_protocols changed: NULL\n");
		}
	} 	

	g_debug("eas_account_set_server_protocols--");
}

GSList*	
eas_account_get_server_protocols (const EasAccount *account)
{
	return account->priv->server_protocols;
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
eas_account_get_calendar_folder (const EasAccount *account)
{
	return account->priv->calendar_folder;
}

gchar*
eas_account_get_contact_folder (const EasAccount *account)
{
	return account->priv->contact_folder;
}

gchar*
eas_account_get_password (const EasAccount *account)
{
	return account->priv->password;
}

gchar*
eas_account_get_device_id (const EasAccount *account)
{
	g_debug("Getting device_id %s", account->priv->device_id);
	return account->priv->device_id;
}

int
eas_account_get_protocol_version (const EasAccount *account)
{
	return account->priv->protocol_version;
}

gboolean
eas_account_set_from_info(EasAccount *account, const EasAccountInfo* accountinfo)
{
	g_debug("eas_account_set_from_info++"); 
	/* account must have a uid*/
	if (!accountinfo->uid)
		return FALSE;

	/* GSettings will set void values to '', so we have to check. */
	if (accountinfo->uid && accountinfo->uid[0])
		eas_account_set_uid (account, accountinfo->uid);
	if (accountinfo->serverUri && accountinfo->serverUri[0])
		eas_account_set_uri (account, accountinfo->serverUri);
	if (accountinfo->username && accountinfo->username[0])
		eas_account_set_username (account, accountinfo->username);
	if (accountinfo->policy_key && accountinfo->policy_key[0])
		eas_account_set_policy_key (account, accountinfo->policy_key);
	if (accountinfo->calendar_folder && accountinfo->calendar_folder[0])
		eas_account_set_calendar_folder(account, accountinfo->calendar_folder);
	if (accountinfo->contact_folder && accountinfo->contact_folder[0])
		eas_account_set_contact_folder(account, accountinfo->contact_folder);
	if (accountinfo->password && accountinfo->password[0])
		eas_account_set_password (account, accountinfo->password);
	if (accountinfo->device_id && accountinfo->device_id[0])
		eas_account_set_device_id (account, accountinfo->device_id);
	eas_account_set_protocol_version (account, accountinfo->protocol_version);
	if (accountinfo->server_protocols && accountinfo->server_protocols)
		eas_account_set_server_protocols (account, accountinfo->server_protocols);
	g_debug("eas_account_set_from_info--");	
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
