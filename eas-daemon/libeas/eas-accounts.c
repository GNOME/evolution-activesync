/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-accounts.h"



G_DEFINE_TYPE (EasAccounts, eas_accounts, G_TYPE_OBJECT);

static void
eas_accounts_init (EasAccounts *object)
{
	/* TODO: Add initialization code here */
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
	// TODO intially to be done as a read from file.
	return "tez";
}

gchar*
eas_accounts_get_password (EasAccounts* self, guint64 accountId)
{
	// TODO intially to be done as a read from file.
	return "M0bica!";
}

gchar*
eas_accounts_get_server_uri (EasAccounts* self, guint64 accountId)
{
	// TODO intially to be done as a read from file.
	return "https://cstylianou.com/Microsoft-Server-ActiveSync";
}
