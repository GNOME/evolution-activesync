/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_UPDATE_EMAIL_REQ_H_
#define _EAS_UPDATE_EMAIL_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_UPDATE_EMAIL_REQ             (eas_update_email_req_get_type ())
#define EAS_UPDATE_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReq))
#define EAS_UPDATE_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqClass))
#define EAS_IS_UPDATE_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_UPDATE_EMAIL_REQ))
#define EAS_IS_UPDATE_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_UPDATE_EMAIL_REQ))
#define EAS_UPDATE_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqClass))

typedef struct _EasUpdateEmailReqClass EasUpdateEmailReqClass;
typedef struct _EasUpdateEmailReq EasUpdateEmailReq;
typedef struct _EasUpdateEmailReqPrivate EasUpdateEmailReqPrivate;

struct _EasUpdateEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasUpdateEmailReq
{
	EasRequestBase parent_instance;

	EasUpdateEmailReqPrivate * priv;
};

GType eas_update_email_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new email update request GObject
 *
 * @param[in] account_id
 *	  Unique identifier for a user account.
 * @param[in] sync_key
 *	  The current synchronisation key.
 * @param[in] folder_id
 *	  The identifer for the target server folder.
 * @param[in] serialised_email_array
 *	  An array of serialised email objects
 * @param[in] context
 *	  A dbus method invocation used to send the completed operation's results
 *	  to the server. Used in MessageComplete
 *
 * @return An allocated EasAddCalendarReq GObject or NULL
 */
EasUpdateEmailReq *eas_update_email_req_new(const gchar* account_id, 
                                            const gchar *sync_key, 
                                            const gchar *folder_id, 
                                            const gchar **serialised_email_array, 
                                            DBusGMethodInvocation *context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasUpdateEmailReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_update_email_req_Activate(EasUpdateEmailReq *self, 
                                       GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then sending the response back across the dbus
 *
 * @param[in] self
 *	  The EasUpdateEmailReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if request is finished and needs cleaning up by connection 
 *    object, otherwise FALSE. 
 */
gboolean eas_update_email_req_MessageComplete(EasUpdateEmailReq *self, 
                                              xmlDoc* doc, 
                                              GError* error);


G_END_DECLS

#endif /* _EAS_UPDATE_EMAIL_REQ_H_ */
