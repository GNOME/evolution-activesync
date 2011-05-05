/*
A draft API for the EAS mail client library which clients will use to get email data from Exchange Active Sync
*/

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>

typedef struct _EasFolder EasFolder;
typedef struct _EasEmail EasEmail;
typedef struct _EasAttachment EasAttachment;

struct _EasFolder {
	gchar *parent_id;
	gchar *server_id;		// from AS server. string up to 64 characters
	gchar *display_name;
};

#define EAS_EMAIL_READ		0x00000001		// whether email has been read
#define EAS_EMAIL_ANSWERED	0x00000002		// not clear how AS supports answered/forwarded! Read-only
#define EAS_EMAIL_FORWARDED	0x00000004		// Read-only

struct _EasEmail {
	gchar *server_id;		// from AS server
	gchar *mime_headers;		// for created email will be a string of mime headers separated by crlfs. Immutable
	GSList *attachments;		// list of EasAttachments this email has. AS calls id the 'file reference'. Immutable
	guint8	flags;			// bitmap. read, answered, forwarded
	GSList *categories;		// list of categories (strings) that the email belongs to 
};

struct _EasAttachment {
	gchar *file_reference;		
	guint	estimated_size;
	gchar *display_name;
	// method;
	// inline;
};


gboolean DBus_Init();

// pulls down changes in folder structure (folders added/deleted/updated). Supplies lists of EasFolders
gboolean SyncFolderHierarchy(gchar *sync_key, 	
			GSList **folders_created,	
			GSList **folders_updated,
			GSList **folders_deleted,
			GError **error);

/* sync emails in a specified folder (no bodies retrieved). 
Returns lists of EasEmails. 
Max changes in one sync = 100 (configurable somewhere?)
In the case of created emails all fields are filled in.
In the case of deleted emails only the serverids are valid. 
In the case of updated emails only the serverids, flags and categories are valid.
*/
gboolean FolderSync(gchar *folder_id,	// folder to sync
	GSList **emails_created,
	GSList **emails_updated,	
	GSList **emails_deleted,
	// guint	window_size,	// max changes we want from server 0..512. AS defaults to 100
	guint	time_filter,		// AS calls filter_type. Eg "sync back 3 days". Required on API?
	gboolean *more_available,	// if there are more changes to sync (window_size exceeded)
	GError **error);


// get the entire email body for listed emails
// each email body will be written to a file with the emailid as its name
gboolean FetchEmailBodies(const GSList *email_ids, 		// emails to fetch. List of EasEmails
		const gchar *mime_directory,
		GError **error);


// get a preview of the email body (up to specified number of characters
gboolean FetchEmailBodyPreviews(const GSList *email_ids, 		// emails to fetch. List of EasEmails
		const gchar *mime_directory,
		guint preview,				// max characters (0..255)
		GError **error);

// 
gboolean FetchEmailAttachments(const GSList *attachments, 	// attachments to fetch - list of file references
		guint max_size,					// max bytes to download for each email
		gboolean allornone,				// whether to retrieve at all if it exceeds max size
		const gchar *mime_directory,			// directory to put attachment files. Filenames will match names in supplied file references 
		GError **error);


// Delete specified emails 
gboolean DeleteEmails(const GSList *emails,		// List of EasEmails to delete
		GError **error);


gboolean DeleteFolderContents(const gchar *folder_id,
			GError **error);

/* 
push email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
*/
gboolean UpdateEmails(GSList *update_emails,		// List of EasEmails to update
				GError **error);


gboolean SendEmail(const gchar *client_email_id,	// unique message identifier supplied by client
	const gchar *mime_file,		// the full path to the email (mime) to be sent
	const gchar *email_account,
	gboolean save_in_sent_folder,
	GError **error);

gboolean ForwardEmail(const gchar *server_folder_id,	// id of the original email's folder (if not INBOX)
	const gchar *server_original_email_id,	// id of the original email
	gboolean edited_inline,		// if true, server will not include the original email text. AS calls 'ReplaceMime'
	const gchar *client_email_id,	// unique message identifier supplied by client for the forwarded email
	const gchar *mime_file,		// the full path to the email (mime) to be forwarded
	const gchar *email_account,
	gboolean save_in_sent_folder,
	GError **error);

gboolean ReplyToEmail(const gchar *server_folder_id,	// id of the original email's folder (if not INBOX)
	const gchar *server_email_id,	// id of the original email
	gboolean edited_inline,		// if true, server will not include the original email text. AS calls 'ReplaceMime'
	const gchar *client_email_id,	// unique message identifier supplied by client
	const gchar *mime_file,		// the full path to the email (mime) to be sent as reply
	const gchar *email_account,
	gboolean save_in_sent_folder,
	GError **error);


gboolean MoveToFolder(const GSList *email_ids,
	const gchar *src_folder_id,
	const gchar *dest_folder_id,
	GError **error);

// How supported in AS?
gboolean CopyToFolder(const GSList *email_ids,
	const gchar *src_folder_id,
	const gchar *dest_folder_id,
	GError **error);

/*
Outstanding issues:
How do drafts work (AS docs say email can't be added to server using sync command with add)?
How does AS expose 'answered' / 'forwarded'? investigate email2:ConversationIndex (timestamps are added when email replied to/forwarded but not clear how to distinguish between those two things?
In order to support answered/forwarded metadata we believe that it's necessary to use SmartReply/SmartForward rather than SendMail.

*/

/* 
API Questions:
Do we need the ability to get multiple attachements at once?
Do we need the ability to sync ALL email folders at once? Assuming not.
*/



#endif
