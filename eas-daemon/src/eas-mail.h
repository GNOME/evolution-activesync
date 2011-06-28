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
	
  	EasMailPrivate* priv;
};

GType eas_mail_get_type (void) G_GNUC_CONST;

/* TODO:Insert your Mail Interface APIS here*/
//START - Test interfaces
gboolean eas_mail_start_sync(EasMail* self, gint valueIn, GError** error);
void eas_mail_test_001(EasMail* self, DBusGMethodInvocation* context);
//END - Test interfaces


EasMail* eas_mail_new(void);

#if 0
void eas_mail_set_eas_connection(EasMail* self, EasConnection* easConnObj);
#endif

/*
	sync the entire email folder hierarchy 
*/                            
gboolean eas_mail_sync_email_folder_hierarchy(EasMail* self,
                                          const gchar* account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context);

/*
	synchronize an email folder. Gets email headers only, not bodies
*/                            
gboolean eas_mail_sync_folder_email(EasMail* easMailObj,
                                    const gchar* account_uid,
									const gchar* sync_key,
									const gchar *collection_id,
                                    DBusGMethodInvocation* context);
/*
    delete an email 
*/
gboolean eas_mail_delete_email(EasMail* easMailObj,
                                const gchar* account_uid,
                                const gchar *sync_key, 
                                const gchar *folder_id,
                                const gchar **server_ids_array,
                                DBusGMethodInvocation* context);
/*
	fetch an email body 
*/
gboolean
eas_mail_fetch_email_body (EasMail* self, 
                           const gchar* account_uid, 
                           const gchar *collection_id, 
                           const gchar *server_id, 
                           const gchar *mime_directory, 
                           DBusGMethodInvocation* context);

/*
	fetch an email attachment
*/
gboolean
eas_mail_fetch_attachment (EasMail* self, 
                          const gchar* account_uid, 
                          const gchar *file_reference,
                          const gchar *mime_directory,
                          DBusGMethodInvocation* context);


/*
	send an email
*/						
gboolean eas_mail_send_email(EasMail* self, 
                             const gchar* account_uid,
                             const gchar* clientid,
                             const gchar *mime_file,
                             DBusGMethodInvocation* context);

/*
	update an email
 */
gboolean eas_mail_update_emails(EasMail *self,
								const gchar* account_uid,
								const gchar *sync_key, 
								const gchar *folder_id,
								const gchar **serialised_email_array,
								DBusGMethodInvocation* context);

/*
	move an email
 */
gboolean eas_mail_move_emails_to_folder (EasMail* easMailObj,
                     const gchar* account_uid,
                     const gchar** server_ids_array,
                     const gchar *src_folder_id,
                     const gchar *dest_folder_id,
                     DBusGMethodInvocation* context);

/*
    watch folders for updates 
*/
gboolean eas_mail_watch_email_folders(EasMail* easMailObj,
                                const gchar* account_uid,
                                const gchar * heartbeat,
                                const gchar **folder_array,
                                DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_MAIL_H_ */
