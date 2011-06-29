/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_SYNC_REQ_H_
#define _EAS_SYNC_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC_REQ             (eas_sync_req_get_type ())
#define EAS_SYNC_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC_REQ, EasSyncReq))
#define EAS_SYNC_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC_REQ, EasSyncReqClass))
#define EAS_IS_SYNC_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC_REQ))
#define EAS_IS_SYNC_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC_REQ))
#define EAS_SYNC_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC_REQ, EasSyncReqClass))

typedef struct _EasSyncReqClass EasSyncReqClass;
typedef struct _EasSyncReq EasSyncReq;
typedef struct _EasSyncReqPrivate EasSyncReqPrivate;

struct _EasSyncReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSyncReq
{
	EasRequestBase parent_instance;
	
	EasSyncReqPrivate *priv;
};

GType eas_sync_req_get_type (void) G_GNUC_CONST;


/** 
 * Create a new item update request GObject
 *
 * @param[in] syncKey
 *	  The current synchronisation key.
 * @param[in] accountId
 *	  Unique identifier for a user account.
 * @param[in] folderId
 *	  The identifer for the target server folder.
 * @param[in] type
 *	  Identifies the type of update item. e.g. Calendar, Contact, Email
 * @param[in] context
 *	  A dbus method invocation used to send the completed operation's results
 *	  to the server. Used in MessageComplete
 *
 * @return An allocated EasAddCalendarReq GObject or NULL
 */
EasSyncReq *eas_sync_req_new (const gchar* syncKey, 
                              const gchar* accountID, 
                              const gchar* folderId, 
                              EasItemType type,
                              DBusGMethodInvocation *context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasUpdateSyncReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_sync_req_Activate (EasSyncReq *self, 
                                GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then sending the response back across the dbus
 *
 * @param[in] self
 *	  The EasSyncReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if request is finished and needs cleaning up by connection 
 *    object, otherwise FALSE. 
 */
gboolean eas_sync_req_MessageComplete (EasSyncReq *self, 
										  xmlDoc* doc, 
										  GError* error);
G_END_DECLS

#endif /* _EAS_SYNC_REQ_H_ */
