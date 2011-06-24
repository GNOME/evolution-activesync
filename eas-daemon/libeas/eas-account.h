

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
	 gchar* password;
} EasAccountInfo;

GType		eas_account_get_type		(void) G_GNUC_CONST;
EasAccount *	eas_account_new			(void);
EasAccount *	eas_account_new_from_info (EasAccountInfo* accountinfo);
void	eas_account_set_uid			(EasAccount *account, const gchar* uid);
void	eas_account_set_uri			(EasAccount *account, const gchar* uri);
void	eas_account_set_username	(EasAccount *account, const gchar* username);
void	eas_account_set_password	(EasAccount *account, const gchar* password);
gboolean	eas_account_set_from_info	(EasAccount *account, const EasAccountInfo* accountinfo);

gchar*	eas_account_get_uid			(const EasAccount *account);
gchar*	eas_account_get_uri			(const EasAccount *account);
gchar*	eas_account_get_username	(const EasAccount *account);
gchar*	eas_account_get_password	(const EasAccount *account);


G_END_DECLS

#endif /* EAS_ACCOUNT_H */
