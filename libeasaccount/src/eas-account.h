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


#ifndef EAS_ACCOUNT_H
#define EAS_ACCOUNT_H

#include <glib-object.h>

/* Standard GObject macros */
#define EAS_TYPE_ACCOUNT \
	(eas_account_get_type ())
#define EAS_ACCOUNT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), EAS_TYPE_ACCOUNT, EasAccount))
#define EAS_ACCOUNT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), EAS_TYPE_ACCOUNT, EasAccountClass))
#define EAS_IS_ACCOUNT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), EAS_TYPE_ACCOUNT))
#define EAS_IS_ACCOUNT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), EAS_TYPE_ACCOUNT))
#define EAS_ACCOUNT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), EAS_TYPE_ACCOUNT, EasAccountClass))

G_BEGIN_DECLS

typedef struct _EasAccount EasAccount;
typedef struct _EasAccountClass EasAccountClass;
typedef struct _EasAccountPrivate EasAccountPrivate;

struct _EasAccount {
	GObject parent;

	EasAccountPrivate* priv;
};

struct _EasAccountClass {
	GObjectClass parent_class;

	void		(*changed)		(EasAccount *account,
						 gint field);

};

typedef struct	_EasAccountInfo
{
	 gchar* uid; 
	 gchar* serverUri;
	 gchar* username;
	 gchar* policy_key;
	 gchar* contact_folder;
	 gchar* calendar_folder;
	 gchar* device_id;
	 gchar* password;
	 int protocol_version;
	 GSList* server_protocols;	
} EasAccountInfo;

typedef enum _eas_account_item_t {
	EAS_ACCOUNT_UID,

	EAS_ACCOUNT_SERVER_URI,
	EAS_ACCOUNT_USERNAME,
	EAS_ACCOUNT_POLICY_KEY,
	EAS_ACCOUNT_PASSWORD,
	EAS_ACCOUNT_CONTACT_FOLDER,
	EAS_ACCOUNT_CALENDAR_FOLDER,
	EAS_ACCOUNT_DEVICE_ID,
	EAS_ACCOUNT_PROTOCOL_VERSION,
	EAS_ACCOUNT_SERVER_PROTOCOLS,
	
} eas_account_item_t;

GType		eas_account_get_type		(void) G_GNUC_CONST;
EasAccount *	eas_account_new			(void);
EasAccount *	eas_account_new_from_info (EasAccountInfo* accountinfo);
void	eas_account_set_uid			(EasAccount *account, const gchar* uid);
void	eas_account_set_uri			(EasAccount *account, const gchar* uri);
void	eas_account_set_username	(EasAccount *account, const gchar* username);
void	eas_account_set_policy_key	(EasAccount *account, const gchar* policy_key);
void	eas_account_set_contact_folder	(EasAccount *account, const gchar* contact_folder);
void	eas_account_set_calendar_folder	(EasAccount *account, const gchar* calendar_folder);
void	eas_account_set_password	(EasAccount *account, const gchar* password);
void	eas_account_set_protocol_version(EasAccount *account, int protocol_version);
void	eas_account_set_device_id	(EasAccount *account, const gchar* device_id);
void 	eas_account_set_server_protocols (EasAccount *account, const GSList* protocols);

gboolean	eas_account_set_from_info	(EasAccount *account, const EasAccountInfo* accountinfo);

gchar*	eas_account_get_uid			(const EasAccount *account);
gchar*	eas_account_get_uri			(const EasAccount *account);
gchar*	eas_account_get_username	(const EasAccount *account);
gchar*	eas_account_get_policy_key	(const EasAccount *account);
gchar*	eas_account_get_contact_folder	(const EasAccount *account);
gchar*	eas_account_get_calendar_folder	(const EasAccount *account);
gchar*	eas_account_get_password	(const EasAccount *account);
int 	eas_account_get_protocol_version(const EasAccount *account);
gchar*	eas_account_get_device_id	(const EasAccount *account);
GSList*	eas_account_get_server_protocols (const EasAccount *account);
EasAccountInfo*	eas_account_get_account_info(const EasAccount *account);


G_END_DECLS

#endif /* EAS_ACCOUNT_H */
