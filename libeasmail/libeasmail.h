/*
A draft API for the EAS mail client library which clients will use to get email data from Exchange Active Sync
*/

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_EMAIL_HANDLER             (eas_mail_handler_get_type ())
#define EAS_EMAIL_HANDLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_EMAIL_HANDLER, EasEmailHandler))
#define EAS_EMAIL_HANDLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_HANDLER, EasEmailHandlerClass))
#define EAS_IS_EMAIL_HANDLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_EMAIL_HANDLER))
#define EAS_IS_EMAIL_HANDLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_EMAIL_HANDLER))
#define EAS_EMAIL_HANDLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_EMAIL_HANDLER, EasEmailHandlerClass))

typedef struct _EasEmailHandlerClass EasEmailHandlerClass;
typedef struct _EasEmailHandler EasEmailHandler;
typedef struct _EasEmailHandlerPrivate EasEmailHandlerPrivate;

struct _EasEmailHandlerClass
{
	GObjectClass parent_class;
};

struct _EasEmailHandler
{
	GObject parent_instance;
	EasEmailHandlerPrivate *priv;
};

GType eas_mail_handler_get_type (void) G_GNUC_CONST;

EasEmailHandler *eas_mail_handler_new(guint64 account_uid);

// pulls down changes in folder structure (folders added/deleted/updated). Supplies lists of EasFolders
// note that each folder has a sync key and the folder *structure* has a separate sync_key
gboolean eas_mail_handler_sync_folder_hierarchy(EasEmailHandler* this, gchar *sync_key, 	
			                                                                        GSList **folders_created,	
			                                                                        GSList **folders_updated,
			                                                                        GSList **folders_deleted,
			                                                                        GError **error);

/* sync email changes from the server for a specified folder (no bodies retrieved). 
Returns lists of EasEmails. 
default Max changes in one sync = 100
In the case of created emails all fields are filled in.
In the case of deleted emails only the serverids are valid. 
In the case of updated emails only the serverids, flags and categories are valid.
*/
gboolean eas_mail_handler_sync_email_info(EasEmailHandler* this, 
                                                                            gchar *sync_key,
                                                                            const gchar *folder_id,	// folder to sync
	                                                                        GSList **emails_created,
	                                                                        GSList **emails_updated,	
	                                                                        GSList **emails_deleted,
	                                                                        // guint	window_size,	// max changes we want from server 0..512. AS defaults to 100
	                                                                        // guint	time_filter,		// AS calls filter_type. Eg "sync back 3 days". Required on API?
	                                                                        gboolean *more_available,	// if there are more changes to sync (window_size exceeded)
	                                                                        GError **error);


// get the entire email body for listed emails
// each email body will be written to a file with the emailid as its name
// directory is expected to exist
gboolean eas_mail_handler_fetch_email_bodies(EasEmailHandler* this, 
                                                const GSList *email_ids, 		// emails to fetch. List of EasEmails
		                                        const gchar *mime_directory,
		                                        GError **error);


// 
gboolean eas_mail_handler_fetch_email_attachments(EasEmailHandler* this, 
                                                        const GSList *attachments, 	// attachments to fetch - list of file references
		                                                guint max_size,					// max bytes to download for each email
		                                                gboolean allornone,				// whether to retrieve at all if it exceeds max size
		                                                const gchar *mime_directory,			// directory to put attachment files. Filenames will match names in supplied file references 
		                                                GError **error);


// Delete specified emails 
gboolean eas_mail_handler_delete_emails(EasEmailHandler* this, 
                                        const GSList *emails,		// List of EasEmails to delete
	                                    GError **error);


gboolean eas_mail_handler_delete_folder_contents(EasEmailHandler* this, 
                                                    const gchar *folder_id,
			                                        GError **error);

/* 
push email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
*/
gboolean eas_mail_handler_update_emails(EasEmailHandler* this, 
                                        GSList *update_emails,		// List of EasEmails to update
				                        GError **error);


gboolean eas_mail_handler_send_email(EasEmailHandler* this, 
                                    const gchar *client_email_id,	// unique message identifier supplied by client
                                    const gchar *mime_file,		// the full path to the email (mime) to be sent
                                    const gchar *email_account,
                                    gboolean save_in_sent_folder,
                                    GError **error);

gboolean eas_mail_handler_move_to_folder(EasEmailHandler* this, 
                                            const GSList *email_ids,
	                                        const gchar *src_folder_id,
	                                        const gchar *dest_folder_id,
	                                        GError **error);

// How supported in AS?
gboolean eas_mail_handler_copy_to_folder(EasEmailHandler* this, 
                                        const GSList *email_ids,
                                        const gchar *src_folder_id,
                                        const gchar *dest_folder_id,
                                        GError **error);


/*
Assumptions:
when a directory is supplied for putting files into, the directory already exists
no requirement for a method to sync ALL email folders at once.
*/


/*
Outstanding issues:
How do drafts work (AS docs say email can't be added to server using sync command with add)?
How does AS expose 'answered' / 'forwarded'? investigate email2:ConversationIndex (timestamps are added when email replied to/forwarded but not clear how to distinguish between those two things?
*/

/* 
API Questions / TODO:
Do we need the ability to get multiple attachments at once?
Should MIME files (with email bodies) be unicode?

// TODO define an 'email config' API (for, eg, window_size, filtertype rather than passing those options over DBus with each sync...)
// TODO add the ability to pass a UID representing the email account
*/


G_END_DECLS

#endif
