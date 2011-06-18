/*
 *  Filename: libeassync.h
 */

#ifndef EAS_SYNC_H
#define EAS_SYNC_H

#include <glib-object.h>
#include "../../eas-daemon/libeas/eas-request-base.h"


G_BEGIN_DECLS

#define EAS_TYPE_SYNC_HANDLER             (eas_sync_handler_get_type ())
#define EAS_SYNC_HANDLER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_HANDLER, EasSyncHandler))
#define EAS_SYNC_HANDLER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_HANDLER, EasSyncHandlerClass))
#define EAS_IS_SYNC_HANDLER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_HANDLER))
#define EAS_IS_SYNC_HANDLER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_HANDLER))
#define EAS_SYNC_HANDLER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_HANDLER, EasSyncHandlerClass))

typedef struct _EasSyncHandlerClass EasSyncHandlerClass;
typedef struct _EasSyncHandler EasSyncHandler;
typedef struct _EasSyncHandlerPrivate EasSyncHandlerPrivate;

struct _EasSyncHandlerClass{
	GObjectClass parent_class;
};

struct _EasSyncHandler{
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
EasSyncHandler *eas_sync_handler_new(guint64 account_uid);

/* function name:               eas_sync_handler_get calendar_items
 * function description:        pulls down changes in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params: 
 * EasSyncHandler* handler (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key (in / out):  use zero for initial hierarchy or saved value returned 
 *                              from exchange server for subsequent sync requests
 * EAS_ITEM_TYPE type (in):		identify the type of data being synchronised
 * gchar *folder_id (in ):      identifier for folder to sync from                              	 
 * GSList **items_created (out): returns a list of EasCalInfo structs that describe
 *                              created items.  If there are no new created items
 *                              this parameter will be unchanged.
 * GSList **items_updated (out): returns a list of EasCalInfo structs that describe
 *                              updated items.  If there are no new updated items
 *                              this parameter will be unchanged.
 * GSList **items_deleted (out): returns a list of strings that show server IDs of 
 *                              deleted items.  If there are no new deleted items
 *                              this parameter will be unchanged.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_get_calendar_items(EasSyncHandler* handler, 
                                                 gchar *sync_key,
                                            	 EasItemType type,
                                   		  		 const gchar* folder_id,
                                                 GSList **items_created,	
                                                 GSList **items_updated,
                                                 GSList **items_deleted,
                                                 GError **error);
                                                 
/* function name:               eas_sync_handler_delete_items
 * function description:        delete items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params: 
 * EasSyncHandler* handler (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key (in / out):  use zero for initial hierarchy or saved value returned 
 *                              from exchange server for subsequent sync requests
 * gchar *folder_id (in ):      identifier for folder to delete items from    
 * GSList *items_deleted (in): provides a list of strings that identify the deleted
 *                              items' server IDs. 
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_delete_items(EasSyncHandler* handler, 
						gchar *sync_key,
						const gchar* folder_id,
						GSList *items_deleted,
						GError **error);

/* function name:               eas_sync_handler_update_items
 * function description:        update items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params: 
 * EasSyncHandler* handler (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key (in / out):  use zero for initial hierarchy or saved value returned 
 *                              from exchange server for subsequent sync requests
 * EAS_ITEM_TYPE type (in):		identify the type of data being synchronised
 * gchar *folder_id (in ):      identifier for folder to update       
 * GSList *items_updated (in): provides a list of EasCalInfo structs that describe
 *                              update items.  If there are no new updated items
 *                              this parameter will be unchanged.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_update_items(EasSyncHandler* handler, 
						gchar *sync_key,
						EasItemType type,
						const gchar* folder_id,
						GSList *items_updated,
						GError **error);

/* function name:               eas_sync_handler_add_items
 * function description:        add items in calendar folder
 * return value:                TRUE if function success, FALSE if error
 * params: 
 * EasSyncHandler* handler (in):  use value returned from eas_sync_hander_new()
 * gchar *sync_key (in / out):  use zero for initial hierarchy or saved value returned 
 *                              from exchange server for subsequent sync requests
 * EAS_ITEM_TYPE type (in):		identify the type of data being added
 * gchar *folder_id (in ):      identifier for folder to add to       
 * GSList *items_added (in): provides a list of EasCalInfo structs that describe
 *                              added items.  If there are no new updated items
 *                              this parameter will be unchanged.
 * GError **error (out):        returns error information if an error occurs.  If no
 *                              error occurs this will unchanged.  This error information
 *                              could be related to errors in this API or errors propagated
 *                              back through underlying layers
*/
gboolean eas_sync_handler_add_items(EasSyncHandler* handler, 
						gchar *sync_key,
						EasItemType type,
						const gchar* folder_id,
						GSList *items_added,
						GError **error);

G_END_DECLS

#endif /* _EAS_SYNC_H_ */
