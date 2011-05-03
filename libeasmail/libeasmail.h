/*
A draft API for the EAS mail client library which clients will use to get email data from Exchange Active Sync
*/

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>


struct EasFolder {
	gchar *parent_id;
	gchar *server_id;		// from AS server. string up to 64 characters
	gchar *display_name;
};

#define EAS_EMAIL_READ		0x00000001		// whether email has been read
#define EAS_EMAIL_ANSWERED	0x00000002		// not clear how AS supports answered/forwarded!
#define EAS_EMAIL_FORWARDED	0x00000004

struct EasEmail {
{
	gchar *server_id;		// from AS server
	gchar *mime_headers;		// for created email will be a string of mime headers separated by crlfs. Immutable
	GSLIst *attachment_ids;		// list of attachments this email has. AS calls id the 'file reference' 
	guint8	flags;			// bitmap. read, answered, forwarded
	GSLIst *categories;		// list of categories (strings) that the email belongs to 
};

// pulls down changes in folder structure (folders added/deleted/updated)
gboolean SyncFolderHierarchy(gchar *sync_key, 	
			GSList **folders_created,
			GSList **folders_updated,
			GSList **folders_deleted,
			GError **error);


/* sync emails in a specified folder (no bodies retrieved). Returns lists of EasEmails. 
In the case of created emails all fields are filled in.
In the case of deleted emails only the serverids are filled in. 
In the case of updated emails only the serverids, flags and categories are filled in.
*/
gboolean FolderSync(gchar *folder_id,	// folder to sync
	GSList **emails_created,
	GSList **emails_updated,	// updated flags or categories
	GSList **emails_deleted,
	// guint	max_changes,		// max changes we want from server 0..512. AS calls window_size. Required on api? AS defaults to 100
	guint	time_filter,		// AS calls filter_type. Eg "sync back 3 days". Required on API?
	gboolean *more_available,
	GError **error);


// get the entire email body for listed emails
// each email body will be written to a file with the emailid as its name
gboolean FetchEmailBodies(const GSList *email_ids, 		// serverids for emails to fetch
		const gchar *mime_directory,
		GError **error);


// get a preview of the email body (up to specified number of characters
gboolean FetchEmailBodyPreviews(const GSList *email_ids, 		// serverids for emails to fetch
		const gchar *mime_directory,
		GSList **emails,				// list of filenames? or we could make filenames == email ids
		guint preview;					// max characters (0..255)
		GError **error);

// 
gboolean FetchEmailAttachments(const GSList *attachment_ids, 	// attachments to fetch
		guint max_size,					// max bytes to download for each email
		gboolean allornone;				// whether to retrieve at all if email exceeds max size
		const gchar *mime_directory,			// directory to put attachment files
		GError **error);


gboolean DeleteEmails(const GSList *email_ids,
		GError **error);


gboolean DeleteFolderContents(const gchar *folder_id,
			GError **error);

/* 
push email updates to server
Note that the only valid changes are to flags / categories (other changes ignored)
*/
gboolean UpdateEmails(GSList *update_emails,
				GError **error);

SendEmail(const gchar *email_id,	// supplied by client in this case
	const gchar *mime_file,		// the full path to the email (mime) to be sent
	const gchar *email_account,
	gboolean save_in_sent_folder,
	GError **error);

MoveToFolder(const GSList *email_ids,
	const gchar *src_folder_id
	const gchar *dest_folder_id,
	GError **error);

// How supported in AS?
CopyToFolder((const GSList *email_ids,
	const gchar *src_folder_id
	const gchar *dest_folder_id,
	GError **error);

/*
Outstanding issues:
How do drafts work (AS docs say email can't be added to server using sync command with add)?
How does AS expose 'answered' / 'forwarded'? investigate email2:ConversationIndex (timestamps are added when email replied to/forwarded but not clear how to distinguish between those two things?
*/

#endif
