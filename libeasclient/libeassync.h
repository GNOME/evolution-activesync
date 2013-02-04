/*
 * ActiveSync client library for calendar/addressbook synchronisation
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

#ifndef EAS_SYNC_H
#define EAS_SYNC_H

#include <glib-object.h>
#include <gio/gio.h>
#include "eas-item-info.h"
#include "eas-errors.h"


G_BEGIN_DECLS

typedef enum {
	EAS_ITEM_NOT_SPECIFIED = 0,
	EAS_ITEM_FOLDER,
	EAS_ITEM_MAIL,
	EAS_ITEM_CALENDAR,    /**< iCalendar 2.0 VEVENT */
	EAS_ITEM_CONTACT,     /**< vCard 3.0 contact */
	EAS_ITEM_TODO,        /**< iCalendar 2.0 VTODO */
	EAS_ITEM_JOURNAL,     /**< iCalendar 2.0 VJOURNAL */

	// Add other items here

	EAS_ITEM_LAST
} EasItemType;


#define EAS_TYPE_SYNC_HANDLER             (eas_sync_handler_get_type ())
#define EAS_SYNC_HANDLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_HANDLER, EasSyncHandler))
#define EAS_SYNC_HANDLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_HANDLER, EasSyncHandlerClass))
#define EAS_IS_SYNC_HANDLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_HANDLER))
#define EAS_IS_SYNC_HANDLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_HANDLER))
#define EAS_SYNC_HANDLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_HANDLER, EasSyncHandlerClass))

typedef struct _EasSyncHandlerClass EasSyncHandlerClass;
typedef struct _EasSyncHandler EasSyncHandler;
typedef struct _EasSyncHandlerPrivate EasSyncHandlerPrivate;


struct _EasSyncHandlerClass {
	GObjectClass parent_class;
};

struct _EasSyncHandler {
	GObject parent_instance;
	EasSyncHandlerPrivate *priv;
};

GType eas_sync_handler_get_type (void) G_GNUC_CONST;
// This method is called once by clients of the libeas plugin for each account.  The method
// takes an ID that identifies the account and returns a pointer to an EasSyncHandler object.
// This object is required to be passed to all subsiquent calls to this interface to identify the
// account and facilitate the interface to the daemon.
// Note: the account_uid is not validated against accounts provisioned on the device as part of
// this call.  This level of validation will be done on subsequent calls that take EasSyncHandler
// as an argument
EasSyncHandler *eas_sync_handler_new (const char* account_uid);

/* function name:               eas_sync_handler_get_folder_list
 * function description:        gets current folder structure of account. Supplies
 *                              Supplies lists of EasFolders.
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasEmailHandler* this (in):  use value returned from eas_sync_hander_new()
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
eas_sync_handler_get_folder_list (EasSyncHandler *self,
				  gboolean force_refresh,
				  GSList **folders,
				  GCancellable *cancellable,
				  GError **error);

/* function name:               eas_sync_handler_get calendar_items
 * function description:        pulls down changes in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasSyncHandler* self (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key_in (in ):  use zero or NULL for initial hierarchy or saved value returned
 *                              from exchange server for subsequent sync requests
 * gchar **sync_key_out (out ):  store value returned from exchange server for subsequent sync requests
 *                           must be freed by caller
 * EAS_ITEM_TYPE type (in):		identify the type of data being synchronised
 * gchar *folder_id (in ):      identifier for folder to sync from
 * GSList **items_created (out): returns a list of  structs that describe
 *                              created items.  If there are no new created items
 *                              this parameter will be unchanged.
 * GSList **items_updated (out): returns a list of  structs that describe
 *                              updated items.  If there are no new updated items
 *                              this parameter will be unchanged.
 * GSList **items_deleted (out): returns a list of strings that show server IDs of
 *                              deleted items.  If there are no new deleted items
 *                              this parameter will be unchanged.
 * gboolean *more_available:    TRUE if there are more changes to sync (window_size exceeded).
 *                              Otherwise FALSE.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will be unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_get_items (EasSyncHandler* self,
				     const gchar *sync_key_in,
				     gchar **sync_key_out,
				     EasItemType type,
				     const gchar* folder_id,
				     GSList **items_created,
				     GSList **items_updated,
				     GSList **items_deleted,
				     gboolean *more_available,   // if there are more changes to sync (window_size exceeded)
				     GError **error);

/* function name:               eas_sync_handler_delete_items
 * function description:        delete items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasSyncHandler* self (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key_in (in ):   sync key of current sync relationship - cannot be zero or NULL
 *
 * gchar **sync_key_out (out ):  store value returned from exchange server for subsequent sync requests
 *                              must be freed by caller
 * EasItemType type             type of item being deleted - allows for default folder to be selected
 * gchar *folder_id (in ):      identifier for folder to delete items from
 * GSList *items_deleted (in): provides a list of strings that identify the deleted
 *                              items' server IDs.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will be unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_delete_items (EasSyncHandler* self,
					const gchar *sync_key_in,
					gchar ** sync_key_out,
					EasItemType type,
					const gchar* folder_id,
					GSList *items_deleted,
					GError **error);

/* function name:               eas_sync_handler_update_items
 * function description:        update items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasSyncHandler* self (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key_in (in ):  sync key of current session - cannot be zero or NULL
 *
 * gchar **sync_key_out (out ):  store value returned from exchange server for subsequent sync requests
 *                           must be freed by caller
 * EAS_ITEM_TYPE type (in):		identify the type of data being synchronised
 * gchar *folder_id (in ):      identifier for folder to update
 * GSList *items_updated (in): provides a list of  structs that describe
 *                              update items.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will be unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_update_items (EasSyncHandler* self,
					const gchar *sync_key_in,
					gchar **sync_key_out,
					EasItemType type,
					const gchar* folder_id,
					GSList *items_updated,
					GError **error);

/* function name:               eas_sync_handler_add_items
 * function description:        add items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params:
 * EasSyncHandler* self (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key_in (in ):  sync key of current session - cannot be zero or NULL
 *
 * gchar **sync_key_out (out ):  store value returned from exchange server for subsequent sync requests
 *                           must be freed by caller
 * EAS_ITEM_TYPE type (in):		identify the type of data being added
 * gchar *folder_id (in ):      identifier for folder to add to
 * GSList *items_added (in): provides a list of  structs that describe
 *                              added items.  If there are no new updated items
 *                              this parameter will be unchanged.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will be unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_add_items (EasSyncHandler* self,
				     const gchar *sync_key_in,
				     gchar **sync_key_out,
				     EasItemType type,
				     const gchar* folder_id,
				     GSList *items_added,
				     GError **error);


/* function name:               eas_sync_handler_fetch_item
 * function description:        fetch an item from a folder
 * return value:                TRUE if function success, FALSE if error
 * params:
 * gchar *folder_id (in )       :folder id where the item resides - use null if using default folders for
 *                               contacts or calendar items
 * gchar *server_id (in )       : server id of the item to be fetched
 * EasItemInfo *item (out)      : pointer to empty item which will be populated on completion
 * EAS_ITEM_TYPE type (in):		identify the type of data being fetched
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will be unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/

gboolean
eas_sync_handler_fetch_item (EasSyncHandler* self,
			     const gchar *folder_id,
			     const gchar *server_id,
			     EasItemInfo* item,
			     EasItemType type,
			     GError **error);

G_END_DECLS

#endif /* _EAS_SYNC_H_ */
