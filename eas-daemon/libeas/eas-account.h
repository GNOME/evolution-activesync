

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

typedef enum _e_account_item_t {
	EAS_ACCOUNT_ID_NAME,
	EAS_ACCOUNT_ID_SERVER_URI,
	EAS_ACCOUNT_ID_USER_NAME,
	EAS_ACCOUNT_ID_PASSWORD,

	EAS_ACCOUNT_ITEM_LAST
} eas_account_item_t;



struct _EasAccount {
	GObject parent;

	 gchar* uid; 
	 gchar* serverUri;
	 gchar* username;
	 gchar* password;
	// gchar *parent_uid;
		
};

struct _EasAccountClass {
	GObjectClass parent_class;

	void		(*changed)		(EasAccount *account,
						 gint field);
};

GType		eas_account_get_type		(void) G_GNUC_CONST;
EasAccount *	eas_account_new			(void);
EasAccount *	eas_account_new_from_xml		(const gchar *xml);
gboolean	eas_account_set_from_xml		(EasAccount *account,
						 const gchar *xml);
void		eas_account_import		(EasAccount *dest,
						 EasAccount *src);
gchar *		eas_account_to_xml		(EasAccount *account);
gchar *		eas_account_uid_from_xml		(const gchar *xml);
const gchar *	eas_account_get_string		(EasAccount *account,
						 eas_account_item_t type);
gint		eas_account_get_int		(EasAccount *account,
						 eas_account_item_t type);
gboolean	eas_account_get_bool		(EasAccount *account,
						 eas_account_item_t type);
void		eas_account_set_string		(EasAccount *account,
						 eas_account_item_t type,
						 const gchar *v_string);
void		eas_account_set_int		(EasAccount *account,
						 eas_account_item_t type,
						 gint v_int);
void		eas_account_set_bool		(EasAccount *account,
						 eas_account_item_t type,
						 gboolean v_bool);
gboolean	eas_account_writable		(EasAccount *account,
						 eas_account_item_t type);
gboolean	eas_account_writable_option	(EasAccount *account,
						 const gchar *protocol,
						 const gchar *option);
G_END_DECLS

#endif /* EAS_ACCOUNT_H */
