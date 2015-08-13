/*
 * ActiveSync client library for email access
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
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

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>
#include <gio/gio.h>
#include "eas-email-info.h"
#include "eas-provision-list.h"
#include "eas-errors.h"
#include "eas-dbus-client.h"

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
typedef struct _EasIdUpdate EasIdUpdate;

struct _EasEmailHandlerClass {
	GObjectClass parent_class;
};

struct _EasEmailHandler {
	GObject parent_instance;
	EasEmailHandlerPrivate *priv;
};

struct _EasIdUpdate {
	gchar *src_id;
	gchar *dest_id;
	gchar *status;	//indicates a problem with the update if not a null/empty string
};

/*
 Free the memory for a EasIdUpdate
*/
void eas_updatedid_free (EasIdUpdate* updated_id);

/*
take the contents of the structure and turn it into a null terminated string
*/
gboolean eas_updatedid_serialise (const EasIdUpdate* updated_id, gchar **result);

GType eas_mail_handler_get_type (void) G_GNUC_CONST;
// This method is called once by clients of the libeasmail plugin for each account.  The method
// takes an ID that identifies the account and returns a pointer to an EasEmailHandler object.
// This object is required to be passed to all subsiquent calls to this interface to identify the
// account and facilitate the interface to the daemon.
// Note: the account_uid is not validated against accounts provisioned on the device as part of
// this call.  This level of validation will be done on subsequent calls that take EasEmailHandler
// as an argument
EasEmailHandler *eas_mail_handler_new (const gchar* account_uid, GError **error);

/* function name:               eas_mail_handler_get_item_estimate
 * function description:        estimates the number of emails to be synchronised for the specified folder
 *
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * gchar *sync_key (in):  		value returned from the previous sync/update/delete request for this folder
 * const gchar *folder_id (in):
 * guint *estimate (out): 		returns the estimated number of items to be synchronised for the specified folder

 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.
*/
gboolean eas_mail_handler_get_item_estimate (EasEmailHandler* self,
					     const gchar *sync_key,
					     const gchar *folder_id,
					     guint *estimate,
					     GError **error);

/* function name:               eas_mail_handler_get_folder_list
 * function description:        gets current folder structure of account. Supplies
 *                              Supplies lists of EasFolders.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * gboolean force_update (in):  check for updates from the server. If FALSE, uses the
 *                              information already cached by the ActiveSync dæmon.
 * GSList **folders (out):      returns a list of EasFolder structs that describe the
 *                              folders on the server.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean
eas_mail_handler_get_folder_list (EasEmailHandler *self,
				  gboolean force_refresh,
				  GSList **folders,
				  GCancellable *cancellable,
				  GError **error);

/* function name:               eas_mail_handler_sync_email_info
 * function description:        sync email changes from the server for a specified folder
 *                              (no bodies retrieved).  Default Max changes in one sync = 100
 *                              as per Exchange default window size.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const gchar *folder_id (in): this value identifies the folder to get the email info from.
 * 								Use the EasFolder->folder_id value in the EasFolder structs
 *                              returned from the eas_mail_handler_get_folder_list() call.
 * GSList **emails_created (out): returns a list of EasEmailInfos structs that describe
 *                              created mails.  If there are no new created mails
 *                              this parameter will be unchanged.  In the case of created emails
 *                              all fields are filled in
 * GSList **emails_updated (out): returns a list of EasEmailInfos structs that describe
 *                              updated mails.  If there are no new updated mails
 *                              this parameter will be unchanged.  In the case of updated emails
 *                              only the serverids, flags and categories are valid
 * GSList **emails_deleted (out): returns a list of EasEmailInfos structs that describe
 *                              deleted mails.  If there are no new deleted mails
 *                              this parameter will be unchanged.  In the case of deleted emails
 *                              only the serverids are valid
 * gboolean *more_available:    TRUE if there are more changes to sync (window_size exceeded).
 *                              Otherwise FALSE.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_sync_folder_email_info (EasEmailHandler* self,
						  gchar *sync_key,
						  const gchar *folder_id,
						  GSList **emails_created,
						  GSList **emails_updated,
						  GSList **emails_deleted,
						  gboolean *more_available,
						  GCancellable *cancellable,
						  GError **error);


/* function name:               eas_mail_handler_fetch_email_body
 * function description:        get the entire email body for specified email.
 *                              The email body will be written to a file whose name
 *                              matches the server_id. The directory is expected to exist
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const gchar *folder_id (in): this value identifies the folder containing the email to get the body from.
 * 								Use the EasFolder->folder_id value in the EasFolder struct.
 * const gchar *server_id (in): this value identifies the email within the folder (identified by
 *                              folder_id) to get the body from.  Use the EasEmailInfo->server_id
 *                              value in the EasEmailInfo struct returned from the call to
 *                              eas_mail_handler_sync_folder_email_info
 * const gchar *mime_directory (in): directory to put email body into. Mime information will be streamed
 *                              to a binary file.  Filename will match name supplied in server_id.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/

gboolean eas_mail_handler_fetch_email_body (EasEmailHandler *self,
					    const gchar *folder_id,
					    const gchar *server_id,
					    const gchar *mime_directory,
					    EasProgressFn progress_fn,
					    gpointer progress_data,
					    GCancellable *cancellable,
					    GError **error);


/* function name:               eas_mail_handler_fetch_email_attachment
 * function description:        get the entire attachment identified by file_reference
 *                              The attachment is created in the directory supplied in
 *                              mime_directory
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const gchar *file_reference (in): this value identifies the attachment to
 *                              fetch.  Use the EasAttachment->file_reference
 *                              in the EasAttachment
 * const gchar *mime_directory (in): directory to put attachment into. Filename
 *                              will match name supplied in file reference
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_fetch_email_attachment (EasEmailHandler* self,
						  const gchar *file_reference,
						  const gchar *mime_directory,
						  EasProgressFn progress_fn,
						  gpointer progress_data,
						  GError **error);

/* function name:               eas_mail_handler_delete_email
 * function description:        delete the email with the specified server id.
 *                              If this method is called on an email not in the "Deleted Items"
 *                              folder then the email is moved to the "Deleted Items" folder.  If
 *                              this method is called on an email in the "Deleted Items" folder
 *                              then the email will be permanently deleted
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * gchar *sync_key (in):  		use value returned from the previous sync/update/delete request on this folder
 * const gchar *folder_id (in): identifies the folder to delete the email from
 * GSList *items_deleted (in):  identifies the specific emails to delete.  This information is in
 *                              the form of server_ids returned from the eas_mail_handler_sync_folder_email_info
 *                              call
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_delete_email (EasEmailHandler* self,
					gchar *sync_key,
					const gchar *folder_id,
					const GSList *items_deleted,
					GCancellable *cancellable,
					GError **error);


/*
'push' email updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
*/
/* function name:               eas_mail_handler_update_emails
 * function description:        allows the user to update the status of specific meta information
 *                              related to an email.  The information that can be modified is limited
 *                              by the flags supported on this interface, currently this is: read/unread,
 *								and the categories
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * gchar *sync_key (in/out):  	use value returned from the previous sync/update/delete request on this folder
 * gchar *folder_id (in):		id of folder that contains email to update
 * const GSList *update_emails (in/out): identifies the emails to update. List of EasEmailInfos.
 *								The 'status' field for an individual EasEmailInfo may be set to reflect a problem with that email if the update is (otherwise) successful
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_update_email (EasEmailHandler* self,
					gchar *sync_key,
					const gchar *folder_id,
					GSList *update_emails,
					GCancellable *cancellable,
					GError **error);


/* function name:               eas_mail_handler_send_email
 * function description:        this method takes an email in the form of a MIME file and sync's it
 *                              to the exchange server.  The email is then sent by the exchange server
 *                              in accordance with the information (To, CC etc) encapsulated by the
 *                              MIME file.
 *                              note: users of this method should not attempt to save the sent email to
 *                              sent item folders or even assume that the send is successfull.  Confirmation
 *                              of a successfull send and organisation of email into sent folders should
 *                              be done through a subsequent call to eas_mail_handler_get_folder_list
 *                              and eas_mail_handler_sync_folder_email_info.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const gchar *client_email_id (in):  use a unique message identifier supplied by
 *                              client of up to 40 chars.
 * const gchar *mime_file (in): the full path to the email (mime) to be sent.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_send_email (EasEmailHandler* self,
				      const gchar *client_email_id,
				      const gchar *mime_file,
				      EasProgressFn progress_fn,
				      gpointer progress_data,
				      GCancellable *cancellable,
				      GError **error);


/* function name:               eas_mail_handler_move_to_folder
 * function description:        this method allows the user to move an email from one
 *                              folder to another.  It is required that the destination
 *                              folder already exists.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const GSList *server_ids (in):identifies the specific emails to move.  This information is in
 *                              the form of server_ids (gchar*) returned from the eas_mail_handler_sync_folder_email_info
 *                              call
 * const gchar *src_folder_id (in): folder id of the folder from which the email will be moved
 * const gchar *dest_folder_id (in): folder id of the folder to which the email will be moved
 * GSList **server_ids_updates (out): when an email is moved between folders its id changes. this is a list of updated ids/status indicating a problem (EasIdUpdates)
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_move_to_folder (EasEmailHandler* self,
					  const GSList *server_ids,
					  const gchar *src_folder_id,
					  const gchar *dest_folder_id,
					  GSList **server_ids_updates,
					  GError **error);

/* function name:               eas_mail_handler_copy_to_folder
 * function description:        this method allows the user to copy an email from one
 *                              folder to another.  It is required that the destination
 *                              folder already exists.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const GSList *server_ids (in): identifies the specific emails to move.  This information is in
 *                              the form of server_ids returned from the eas_mail_handler_sync_folder_email_info
 *                              call
 * const gchar *src_folder_id (in): folder id of the folder from which the email will be copied
 * const gchar *dest_folder_id (in): folder id of the folder to which the email will be copied
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_copy_to_folder (EasEmailHandler* self,
					  const GSList *server_ids,
					  const gchar *src_folder_id,
					  const gchar *dest_folder_id,
					  GError **error);

/* function callback  prototype for Push Email.
 * params:
 * GSList * data                A list of strings which provide the id's of folders which have
 *                              been updated. Sync these folders before re-issuing watch command.
 * GError * error               error information if an error occurs.  If no
 *                              error occurs this will be NULL.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
typedef void (*EasPushEmailCallback) (GSList* data, GError *error);

/* function name:               eas_mail_handler_watch_email_folder
 * function description:        this method allows the user to watch a number of email
 *                              folders. Function will return when something has changed
 *                              in one of the folders, and will include a list of folders
 *                              which need syncing
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_mail_hander_new()
 * const GSList *folder_ids (in):identifies the specific folders to watch.  This information is in
 *                              the form of folder_ids (gchar*) returned from the eas_mail_handler_get_folder_list
 *                              call
 * const gchar *heartbeat (in): time between ping calls
 * EasPushEmailCallback (in):   callback function to be used to signal when a folder has been changed
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_mail_handler_watch_email_folders (EasEmailHandler* self,
					       const GSList *folder_ids,
					       const gchar *heartbeat,
					       EasPushEmailCallback cb,
					       GError **error);

/* function name:               eas_mail_handler_sync_email_info
 * function description:        sync email with the server for a specified folder
 *                              (no bodies retrieved).  Default Max changes in one sync = 100
 *                              as per Exchange default window size.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  	use value returned from eas_mail_hander_new()
 * const gchar *sync_key_in (in):	use value returned from the previous sync request on this folder (or 0 if this is the first sync)
 * guint	time_window (in):		will retrieve only emails dated within the specified window. valid values are 0-5
 *	 								0 = no window - sync all items
 *	 								1 = 1 day back
 *	 								2 = 3 days back
 *	 								3 = 1 week back
 *	 								4 = 2 weeks back
 *	 								5 = 1 month back
 * const gchar *folder_id (in): 	this value identifies the folder to get the email info from.
 * 									Use the EasFolder->folder_id value in the EasFolder structs
 *                              	returned from the eas_mail_handler_get_folder_list() call.
 * GSList *delete_emails	(in):	list of email's server ids to delete
 * GSList *change_emails 	(in/out):	list of emails to update. In the case of an unsuccessful update the status will be set to indicate the problem
 * gchar *sync_key_out	 	(out):	updated sync key
 * GSList **emails_created (out): 	returns a list of EasEmailInfos structs that describe
 *                              	created mails.  If there are no new created mails
 *                              	this parameter will be unchanged.  In the case of created emails
 *                              	all fields are filled in
 * GSList **emails_updated (out): 	returns a list of EasEmailInfos structs that describe
 *                              	updated mails.  If there are no new updated mails
 *                              	this parameter will be unchanged.  In the case of updated emails
 *                              	only the serverids, flags and categories are valid
 * GSList **emails_deleted (out): 	returns a list of EasEmailInfos structs that describe
 *                              	deleted mails.  If there are no new deleted mails
 *                              	this parameter will be unchanged.  In the case of deleted emails
 *                              	only the serverids are valid
 * gboolean *more_available:    	TRUE if there are more changes to sync (window_size exceeded).
 *                              	Otherwise FALSE.
 * GError **error (out):        	returns error information if an error occurs.  If no
 *                              	error occurs this will unchanged.  This error information
 *                              	could be related to errors in this API or errors propagated
 *                              	back through underlying layers
*/
gboolean eas_mail_handler_sync_folder_email (EasEmailHandler* self,
					     const gchar *sync_key_in,
					     guint	time_window,
					     const gchar *folder_id,
					     GSList *delete_emails,
					     GSList *change_emails,
					     gchar **sync_key_out,
					     GSList **emails_created,
					     GSList **emails_changed,
					     GSList **emails_deleted,
					     gboolean *more_available,
					     EasProgressFn progress_fn,
					     gpointer progress_data,
					     GCancellable *cancellable,
					     GError **error);

/* function name:               eas_mail_handler_get_provision_list
 * function description:        Sends a provisioning request to the server, and
 *                              receives 2 tokens a list of provisioning requirements.
 *                              If the list of requirements it accepted, this is indicated
 *                              to the server by calling eas_mail_handler_accept_provision_list
 *                              using the 2 tokens retrieved in this call.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  	use value returned from eas_mail_hander_new()
 * gchar **tid (out):             	temporary ID token to be used in eas_mail_handler_accept_provision_list
 * gchar **tid_status (out):      	temporary ID status token to be used in eas_mail_handler_accept_provision_list
 * EasProvisionList** provision_list (out): 	a list of provisioning information.
 * GError **error (out):        	returns error information if an error occurs.  If no
 *                              	error occurs this will unchanged.  This error information
 *                              	could be related to errors in this API or errors propagated
 *                              	back through underlying layers
 */
gboolean
eas_mail_handler_get_provision_list (EasEmailHandler *self,
				     gchar** tid,
				     gchar** tid_status,
				     EasProvisionList** provision_list,
				     GCancellable *cancellable,
				     GError **error);

/* function name:               eas_mail_handlerautodiscover
 * function description:        Sends an autodiscover request to the daemon, and
 *                              receives a uri of the server for the account provided.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  	use value returned from eas_mail_hander_new()
 * gchar *email (in):             	email address to for which a server URI is required
 * gchar *username (in):        	username of the account for which a server UIR is required
 * gchar** uri (out):           	uri of server, if found
 * GError **error (out):        	returns error information if an error occurs.  If no
 *                              	error occurs this will unchanged.  This error information
 *                              	could be related to errors in this API or errors propagated
 *                              	back through underlying layers
 */
gboolean
eas_mail_handler_autodiscover (EasEmailHandler *self,
			       const gchar* email,
			       const gchar* username,
			       gchar** uri,
			       GCancellable *cancellable,
			       GError **error);

/* function name:               eas_mail_handler_accept_provision_list
 * function description:        Sends a provisioning acceptance the server, taking
 *                              the 2 tokens retrieved during a previous call to
 *                              eas_mail_handler_get_provision_list.
 *                              A side effect of this function is that the final
 *                              provisioning ID (PID) is written into Gsettings for the account.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  	use value returned from eas_mail_hander_new()
 * const gchar *tid (in):       	temporary ID token retrieved in a previous call to eas_mail_handler_get_provision_list
 * const gchar *tid_status (in):	temporary ID status token retrieved in a previous call to eas_mail_handler_get_provision_list
 * GError **error (out):        	returns error information if an error occurs. If no
 *                              	error occurs this will unchanged.  This error information
 *                              	could be related to errors in this API or errors propagated
 *                              	back through underlying layers
 */
gboolean
eas_mail_handler_accept_provision_list (EasEmailHandler *self,
					const gchar* tid,
					const gchar* tid_status,
					GCancellable *cancellable,
					GError **error);


/*
Outstanding issues:
How do drafts work (AS docs say email can't be added to server using sync command with add)?
How does AS expose 'answered' / 'forwarded'? investigate email2:ConversationIndex (timestamps are added when email replied to/forwarded but not clear how to distinguish between those two things?
*/

/*
API Questions / todos:

// TODO define an 'email config' API (for, eg, window_size, filtertype etc rather than passing those options over DBus with each sync...)
*/


G_END_DECLS

#endif
