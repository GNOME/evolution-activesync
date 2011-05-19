/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_ACCOUNTS_H_
#define _EAS_ACCOUNTS_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_ACCOUNTS             (eas_accounts_get_type ())
#define EAS_ACCOUNTS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ACCOUNTS, EasAccounts))
#define EAS_ACCOUNTS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ACCOUNTS, EasAccountsClass))
#define EAS_IS_ACCOUNTS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ACCOUNTS))
#define EAS_IS_ACCOUNTS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_ACCOUNTS))
#define EAS_ACCOUNTS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_ACCOUNTS, EasAccountsClass))

typedef struct _EasAccountsClass EasAccountsClass;
typedef struct _EasAccounts EasAccounts;

struct _EasAccountsClass
{
	GObjectClass parent_class;
};

struct _EasAccounts
{
	GObject parent_instance;
};

GType eas_accounts_get_type (void) G_GNUC_CONST;
EasAccounts* eas_accounts_new (void);
gchar* eas_accounts_get_user_id (EasAccounts* self, guint64 accountId);
gchar* eas_accounts_get_password (EasAccounts* self, guint64 accountId);
gchar* eas_accounts_get_server_uri (EasAccounts* self, guint64 accountId);

G_END_DECLS

#endif /* _EAS_ACCOUNTS_H_ */
