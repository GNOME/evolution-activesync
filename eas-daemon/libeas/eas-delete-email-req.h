/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_DELETE_EMAIL_REQ_H_
#define _EAS_DELETE_EMAIL_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_DELETE_EMAIL_REQ             (eas_delete_email_req_get_type ())
#define EAS_DELETE_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReq))
#define EAS_DELETE_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqClass))
#define EAS_IS_DELETE_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_DELETE_EMAIL_REQ))
#define EAS_IS_DELETE_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_DELETE_EMAIL_REQ))
#define EAS_DELETE_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_DELETE_EMAIL_REQ, EasDeleteEmailReqClass))

typedef struct _EasDeleteEmailReqClass EasDeleteEmailReqClass;
typedef struct _EasDeleteEmailReq EasDeleteEmailReq;
typedef struct _EasDeleteEmailReqPrivate EasDeleteEmailReqPrivate;

struct _EasDeleteEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasDeleteEmailReq
{
	EasRequestBase parent_instance;

	EasDeleteEmailReqPrivate* priv;	
};

GType eas_delete_email_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new delete email request GObject
 *
 * @param[in] account_id
 *	  Unique identifier for a user account.
 * @param[in] sync_key
 *	  The current synchronisation key.
 * @param[in] folder_id
 *	  The identifer for the target server folder.
 * @param[in] server_ids_array
 *	  A list of email item server ids to be deleted.
 * @param[in] flag
 *	  A semaphore used to make the request appear synchronous by waiting for the
 *	  server response. It should be set by the caller immediately after this 
 *	  function is called and cleared in this request's MessageComplete.
 *
 * @return An allocated EasAddCalendarReq GObject or NULL
 */
EasDeleteEmailReq *eas_delete_email_req_new (const gchar* accountId, 
                                             const gchar *syncKey, 
                                             const gchar *folderId, 
                                             const GSList *server_ids_array, 
                                             DBusGMethodInvocation *context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasDeleteEmailReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_delete_email_req_Activate (EasDeleteEmailReq *self, 
                                        GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then returning the results across the dbus to the client *
 * @param[in] self
 *	  The EasDeleteEmailReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 * @return TRUE if finished and needs unreffing, FALSE otherwise
 */
gboolean eas_delete_email_req_MessageComplete (EasDeleteEmailReq *self, 
                                           xmlDoc* doc, 
                                           GError* error);

G_END_DECLS

#endif /* _EAS_DELETE_EMAIL_REQ_H_ */
