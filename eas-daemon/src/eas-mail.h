/*
 * Filename: eas-mail.h
 */

#ifndef _EAS_MAIL_H_
#define _EAS_MAIL_H_
#include <dbus/dbus-glib.h>
#include <glib-object.h>
#include "../libeas/eas-connection.h"

G_BEGIN_DECLS

#define EAS_TYPE_MAIL             (eas_mail_get_type ())
#define EAS_MAIL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_MAIL, EasMail))
#define EAS_MAIL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_MAIL, EasMailClass))
#define EAS_IS_MAIL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_MAIL))
#define EAS_IS_MAIL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_MAIL))
#define EAS_MAIL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_MAIL, EasMailClass))

typedef struct _EasMailClass EasMailClass;
typedef struct _EasMail EasMail;
typedef struct _EasMailPrivate EasMailPrivate;


struct _EasMailClass
{
	GObjectClass parent_class;
};

struct _EasMail
{
	GObject parent_instance;
	
  	EasMailPrivate* _priv;
};

GType eas_mail_get_type (void) G_GNUC_CONST;

/* TODO:Insert your Mail Interface APIS here*/
//START - Test interfaces
gboolean eas_mail_start_sync(EasMail* self, gint valueIn, GError** error);
void eas_mail_test_001(EasMail* self, DBusGMethodInvocation* context);
//END - Test interfaces


EasMail* eas_mail_new(void);

void eas_mail_set_eas_connection(EasMail* self, EasConnection* easConnObj);

EasConnection*  eas_mail_get_eas_connection(EasMail* self);

/*
	sync the entire email folder hierarchy 
*/                            
void eas_mail_sync_email_folder_hierarchy(EasMail* self,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context);

/*
	synchronize an email folder. Gets email headers only, not bodies
*/                            
gboolean eas_mail_sync_folder_email(EasMail* easMailObj,
                                    guint64 account_uid,
									const gchar* sync_key,
									const gchar *collection_id,
                                    DBusGMethodInvocation* context);
/*
    delete an email 
*/
gboolean eas_mail_delete_email(EasMail* easMailObj,
                                    const gchar* sync_key, 
                                    const gchar *server_id,
                                    GError **error);
/*
	fetch an email body or attachment
*/
gboolean
eas_mail_fetch (EasMail* self, 
                guint64 account_uid, 
                const gchar *server_id, 
                const gchar *collection_id, 
                const gchar *file_reference, 
                const gchar *mime_directory, 
                GError **error);

/*
	send an email
*/						
gboolean eas_mail_send_email(EasMail* self, 
                             	guint64 account_uid,
								const gchar* clientid,
								const gchar *mime_file,
								GError** error);								
											

G_END_DECLS

#endif /* _EAS_MAIL_H_ */
