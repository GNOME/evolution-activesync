/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */
#include <stdio.h>
#include "eas-accounts.h"

G_DEFINE_TYPE (EasAccounts, eas_accounts, G_TYPE_OBJECT);

struct	_EasAccountsPrivate
{
	 gint64 accountId;
	 gchar serverUri[64];
	 gchar username[64];
	 gchar password[64]; 
};
	
#define EAS_ACCOUNTS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ACCOUNTS, EasAccountsPrivate))

static void
eas_accounts_init (EasAccounts *object)
{
	EasAccountsPrivate *priv =NULL;
	object->_priv = priv = EAS_ACCOUNTS_PRIVATE(object); 

	priv->accountId = 0;
}

static void
eas_accounts_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_accounts_parent_class)->finalize (object);
}

static void
eas_accounts_class_init (EasAccountsClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_accounts_finalize;
	
	g_type_class_add_private (klass, sizeof (EasAccountsPrivate));
	
}


EasAccounts*
eas_accounts_new (void)
{
	EasAccounts* accounts;

	accounts = g_object_new(EAS_TYPE_ACCOUNTS, NULL);

	return accounts;
}

gchar*
eas_accounts_get_user_id (EasAccounts* self, guint64 accountId)
{
	return   self->_priv->username;
}

gchar*
eas_accounts_get_password (EasAccounts* self, guint64 accountId)
{
	return self->_priv-> password;
}

gchar*
eas_accounts_get_server_uri (EasAccounts* self, guint64 accountId)
{
	return self->_priv->serverUri;
}

int eas_accounts_read_accounts_info(EasAccounts* self)
{
	printf("eas_accounts_read_accounts_info ++\n");	 
	FILE *myfile =NULL;
	myfile = fopen("../../eas-daemon/data/accounts.cfg","r");
   if(myfile==NULL)
   {
	    fprintf(stderr, "Can't open input file accounts.cfg!\n");
 		 return 1;   
   }
   	
	int status = 0;
	status = fscanf(myfile, "accountId=%lld\nserverUri=%s\nusername=%s\npassword=%s\n",
						      &self->_priv->accountId, 	self->_priv->serverUri, self->_priv->username, self->_priv->password);
  if (status != 4)
   {
   	    fprintf(stderr, "Error reading data from file accounts.cfg!\n");
   	    fclose (myfile);
		return 2;
   }

	printf("account=%lld\nserverUri=%s\nusername=%s\npassword=%s\n",
							    self->_priv->accountId, 	self->_priv->serverUri, self->_priv->username, self->_priv->password);


	 fclose (myfile);
	printf("eas_accounts_read_accounts_info --\n");	 
	return 0;
}


