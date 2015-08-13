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

#ifndef __EAS_ACCOUNT_LIST__
#define __EAS_ACCOUNT_LIST__


#include <libedataserver/eds-version.h>
#if EDS_CHECK_VERSION(3,6,0)
# include <libedataserver/libedataserver.h>
#else
# include <libedataserver/e-list.h>
#endif

#include "eas-account.h"
#include <gio/gio.h>

#define EAS_ACCOUNT_ROOT			"/apps/activesyncd/accounts"
#define EAS_ACCOUNT_KEY_SERVERURI	"serveruri"
#define EAS_ACCOUNT_KEY_USERNAME	"username"
#define EAS_ACCOUNT_KEY_POLICY_KEY	"policy-key"
#define EAS_ACCOUNT_KEY_CONTACT_FOLDER	"contact-folder"
#define EAS_ACCOUNT_KEY_CALENDAR_FOLDER	"calendar-folder"
#define EAS_ACCOUNT_KEY_PASSWORD	"password"
#define EAS_ACCOUNT_KEY_DEVICE_ID	"device-id"
#define EAS_ACCOUNT_KEY_PROTOCOL_VERSION "protocol-version"
#define EAS_ACCOUNT_KEY_SERVER_PROTOCOLS "server-protocols"

/* Standard GObject macros */
#define EAS_TYPE_ACCOUNT_LIST \
	(eas_account_list_get_type ())
#define EAS_ACCOUNT_LIST(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), EAS_TYPE_ACCOUNT_LIST, EasAccountList))
#define EAS_ACCOUNT_LIST_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), EAS_TYPE_ACCOUNT_LIST, EasAccountListClass))
#define EAS_IS_ACCOUNT_LIST(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), EAS_TYPE_ACCOUNT_LIST))
#define EAS_IS_ACCOUNT_LIST_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), EAS_TYPE_ACCOUNT_LIST))
#define EAS_ACCOUNT_LIST_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), EAS_TYPE_ACCOUNT_LIST, EasAccountListClass))

G_BEGIN_DECLS

typedef struct _EasAccountList EasAccountList;
typedef struct _EasAccountListClass EasAccountListClass;
typedef struct _EasAccountListPrivate EasAccountListPrivate;

/* search options for the find command */
typedef enum _eas_account_find_t {
	EAS_ACCOUNT_FIND_ACCOUNT_UID,
	EAS_ACCOUNT_FIND_SERVER_URI,
	EAS_ACCOUNT_FIND_USER_NAME,
	EAS_ACCOUNT_FIND_POLICY_KEY,
	EAS_ACCOUNT_FIND_CALENDAR_FOLDER,
	EAS_ACCOUNT_FIND_CONTACT_FOLDER,
	EAS_ACCOUNT_FIND_PASSWORD,
} eas_account_find_t;

/**
 * EasAccountList:
 *
 * Contains only private data that should be read and manipulated using the
 * functions below.
 **/
struct _EasAccountList {
	EList parent;
	EasAccountListPrivate *priv;
};

struct _EasAccountListClass {
	EListClass parent_class;

	/* signals */
	void		(*account_added)	(EasAccountList *account_list,
						 EasAccount *account);
	void		(*account_changed)	(EasAccountList *account_list,
						 EasAccount *account);
	void		(*account_removed)	(EasAccountList *account_list,
						 EasAccount *account);
};

GType		eas_account_list_get_type		(void) G_GNUC_CONST;
EasAccountList *	eas_account_list_new		(GSettings *setting);
void		eas_account_list_construct	(EasAccountList *account_list,
						 GSettings *setting);
void		eas_account_list_save_list		(EasAccountList *account_list);

void		eas_account_list_add		(EasAccountList *account_list,
						 EasAccount *account);
void		eas_account_list_change		(EasAccountList *account_list,
						 EasAccount *account);
void		eas_account_list_remove		(EasAccountList *account_list,
						 EasAccount *account);
EasAccount *eas_account_list_find		(EasAccountList *account_list,
						 eas_account_find_t type,
						 const gchar *key);

/* APIs for handling individual accounts and account fields*/
void 		eas_account_list_save_account(EasAccountList *account_list,
						EasAccount *account);
void 		eas_account_list_save_item(EasAccountList *account_list,
						EasAccount *account, eas_account_item_t type);
G_END_DECLS

#endif /* __EAS_ACCOUNT_LIST__ */
