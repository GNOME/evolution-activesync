

#ifndef __EAS_ACCOUNT_LIST__
#define __EAS_ACCOUNT_LIST__


#include <libedataserver/e-list.h>

#include "eas-account.h"
#include <gconf/gconf-client.h>

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
typedef enum _e_account_find_t {
	EAS_ACCOUNT_FIND_ACCOUNT_UID,
	EAS_ACCOUNT_FIND_SERVER_URI,
	EAS_ACCOUNT_FIND_USER_NAME,
	EAS_ACCOUNT_FIND_POLICY_KEY,
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
EasAccountList *	eas_account_list_new		(GConfClient *client);
void		eas_account_list_construct	(EasAccountList *account_list,
						 GConfClient *client);
void		eas_account_list_save_list		(EasAccountList *account_list);

void		eas_account_list_add		(EasAccountList *account_list,
						 EasAccount *account);
void		eas_account_list_change		(EasAccountList *account_list,
						 EasAccount *account);
void		eas_account_list_remove		(EasAccountList *account_list,
						 EasAccount *account);
const EasAccount *eas_account_list_find		(EasAccountList *account_list,
						 eas_account_find_t type,
						 const gchar *key);

/* APIs for handling individual accounts and account fields*/
void 		eas_account_list_save_account(EasAccountList *account_list,
						EasAccount *account);
void 		eas_account_list_save_item(EasAccountList *account_list,
						EasAccount *account, eas_account_item_t type);
G_END_DECLS

#endif /* __EAS_ACCOUNT_LIST__ */
