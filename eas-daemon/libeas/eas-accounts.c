/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 *
 */
#include <stdio.h>
#include "eas-accounts.h"

G_DEFINE_TYPE (EasAccounts, eas_accounts, G_TYPE_OBJECT);

struct  _EasAccountsPrivate
{
    gchar accountUid[64];
    gchar serverUri[128];
    gchar username[64];
    gchar password[64];
};

#define EAS_ACCOUNTS_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_ACCOUNTS, EasAccountsPrivate))

static void
eas_accounts_init (EasAccounts *object)
{
    EasAccountsPrivate *priv = NULL;
    object->_priv = priv = EAS_ACCOUNTS_PRIVATE (object);
}

static void
eas_accounts_finalize (GObject *object)
{
    EasAccounts* accounts = (EasAccounts *) object;

    g_free (accounts->_priv->accountUid);

    G_OBJECT_CLASS (eas_accounts_parent_class)->finalize (object);
}

static void
eas_accounts_class_init (EasAccountsClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);
    void *tmp = parent_class;
    tmp = object_class;

    object_class->finalize = eas_accounts_finalize;

    g_type_class_add_private (klass, sizeof (EasAccountsPrivate));

}


EasAccounts*
eas_accounts_new (void)
{
    EasAccounts* accounts;

    accounts = g_object_new (EAS_TYPE_ACCOUNTS, NULL);

    return accounts;
}

gchar*
eas_accounts_get_user_id (EasAccounts* self, const gchar* accountUid)
{
    return self->_priv->username;
}

gchar*
eas_accounts_get_password (EasAccounts* self, const gchar* accountUid)
{
    return self->_priv-> password;
}

gchar*
eas_accounts_get_server_uri (EasAccounts* self, const gchar* accountUid)
{
    return self->_priv->serverUri;
}

int eas_accounts_read_accounts_info (EasAccounts* self)
{
    FILE *file = NULL;
    int status = 0;
    g_debug ("eas_accounts_read_accounts_info++");

    file = fopen ("/usr/local/etc/accounts.cfg", "r");
    if (file == NULL)
    {
        g_debug ("Can't find config file - need to copy data/accounts.cfg to /usr/local/etc");
        return 1;
    }

    status = fscanf (file, "accountId=%s\nserverUri=%s\nusername=%s\npassword=%s\n",
                     self->_priv->accountUid, self->_priv->serverUri,
                     self->_priv->username, self->_priv->password);
    if (status != 4)
    {
        g_debug ("Error reading data from file accounts.cfg!");
        fclose (file);
        return 2;
    }

    g_debug ("account=%s\nserverUri=%s\nusername=%s\npassword=%s",
             (self->_priv->accountUid ? self->_priv->accountUid : "NULL"),
             self->_priv->serverUri,
             self->_priv->username,
             self->_priv->password);


    fclose (file);
    g_debug ("eas_accounts_read_accounts_info--");
    return 0;
}


