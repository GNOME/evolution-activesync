/*
A draft API for the EAS mail client library which clients will use to get email data from Exchange Active Sync
*/

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>


struct EasFolder {
	gchar *parent__id;
	gchar *server_id;		// from AS server
	gchar *display_name;
};

struct EasEmail {
{
	gchar *server_id;		// from AS server
	gchar *mime_headers;		// for created email will be a string of mime headers separated by crlfs. 
	GSLIst *attachment_ids;		
	gboolean read;
	GSLIst *categories;		// array of category strings
};


//
gboolean SyncFolderHierarchy(gchar *sync_key, 	
			GSList **folders_created,
			GSList **folders_updated,
			GSList **folders_deleted,
			GError **error);


// sync emails in a folder (no bodies retrieved)
gboolean FolderSync(guint max_entries,
	gboolean *more_available,
	GSList **emails_created,
	GSList **emails_updated,		// updated read/unread or categories
	GSList **emails_deleted,
	guint	max_changes,			// max changes we want from server. need to support now? AS calls window_size
	guint	time_filter,			// AS calls filter_type. Eg "sync back 3 days"
	gboolean *more_available,
	GError **error);


// get the entire email body for listed emails
gboolean FetchEmailBodies(const GSList *email_ids, 		// serverids for emails to fetch
		const gchar *mime_directory,
		GSList **emails,				// list of filenames? or we could make filenames == email ids
		GError **error);


// get a preview of the email body (up to specified number of characters
gboolean FetchEmailBodyPreviews(const GSList *email_ids, 		// serverids for emails to fetch
		const gchar *mime_directory,
		GSList **emails,				// list of filenames? or we could make filenames == email ids
		guint preview;					// max characters (0..255)
		GError **error);

// allow retrieval of attachments for multiple emails at once?
gboolean FetchEmailAttachments(const GSList *attachment_ids, 		// email we want attachments for
		GSList **attachments,				// list of filenames?
		guint max_size,					// max bytes to download for each email
		gboolean allornone;				// whether to retrieve at all if email exceeds max size
		const gchar *mime_directory,
		GError **error);


gboolean DeleteEmails(const GSList *email_ids,
		GError **error);


gboolean DeleteFolderContents(const gchar *folder_id,
			GError **error);

gboolean UpdateEmailRead(const GSList *email_ids, gboolean read,
		GError **error);

gboolean UpdateEmailImportance(const GSList *email_ids, guint importance,
		GError **error);

// supported on AS?
/*
gboolean UpdateEmailJunk(const GSList *email_ids, gboolean junk,
		GError **error);
*/

// supported on AS?
/*
gboolean UpdateEmailFollowup(const GSList *email_ids, gboolean followup,
		GError **error);
*/

// 'Label' in Evolution
// 
UpdateEmailCategories(const GSList *email_ids, 
		const gchar** categories, 		// array of strings for categories to add email to	
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
How do we implement 'junk' / 'followup' in AS?

*/

#endif
