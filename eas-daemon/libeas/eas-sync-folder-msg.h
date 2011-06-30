/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_SYNC_FOLDER_MSG_H_
#define _EAS_SYNC_FOLDER_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_FOLDER_MSG             (eas_sync_folder_msg_get_type ())
#define EAS_SYNC_FOLDER_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsg))
#define EAS_SYNC_FOLDER_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))
#define EAS_IS_SYNC_FOLDER_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_IS_SYNC_FOLDER_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_FOLDER_MSG))
#define EAS_SYNC_FOLDER_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_FOLDER_MSG, EasSyncFolderMsgClass))

typedef struct _EasSyncFolderMsgClass EasSyncFolderMsgClass;
typedef struct _EasSyncFolderMsg EasSyncFolderMsg;
typedef struct _EasSyncFolderMsgPrivate EasSyncFolderMsgPrivate;

struct _EasSyncFolderMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasSyncFolderMsg
{
	EasMsgBase parent_instance;

	EasSyncFolderMsgPrivate *priv;
};

GType eas_sync_folder_msg_get_type (void) G_GNUC_CONST;

/**
 * Create a new email body message.
 *
 * @param[in] accountId
 *	  Unique identifier for a user account.
 * @param[in] syncKey
 *	  The client's current syncKey.
 *
 * @return NULL or a newly created EasSyncFolderMsg GObject.
 */
EasSyncFolderMsg* eas_sync_folder_msg_new (const gchar* syncKey, const gchar* accountId);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* eas_sync_folder_msg_build_message (EasSyncFolderMsg* self);

/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasSyncFolderMsg GObject instance was 
 * created.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 * @param[in] doc
 *	  libxml DOM tree structure containing the XML to be parsed. [no transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_sync_folder_msg_parse_response (EasSyncFolderMsg* self, 
                                             const xmlDoc *doc, 
                                             GError** error);

/**
 * Retrieves the response added folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been added.
 */
GSList* eas_sync_folder_msg_get_added_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the response updated folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been updated.
 */
GSList* eas_sync_folder_msg_get_updated_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the response deleted folders.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or List of EasFolder GObjects that have been deleted.
 */
GSList* eas_sync_folder_msg_get_deleted_folders (EasSyncFolderMsg* self);

/**
 * Retrieves the sync key in the response from the server.
 *
 * @param[in] self
 *	  The EasSyncFolderMsg GObject instance.
 *
 * @return NULL or The updated sync key supplied by the server response. [no transfer]
 */
gchar* eas_sync_folder_msg_get_syncKey(EasSyncFolderMsg* self);

G_END_DECLS

#endif /* _EAS_SYNC_FOLDER_MSG_H_ */
