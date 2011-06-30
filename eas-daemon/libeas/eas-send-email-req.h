/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_SEND_EMAIL_REQ_H_
#define _EAS_SEND_EMAIL_REQ_H_

#include <glib-object.h>

#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SEND_EMAIL_REQ             (eas_send_email_req_get_type ())
#define EAS_SEND_EMAIL_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReq))
#define EAS_SEND_EMAIL_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqClass))
#define EAS_IS_SEND_EMAIL_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SEND_EMAIL_REQ))
#define EAS_IS_SEND_EMAIL_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SEND_EMAIL_REQ))
#define EAS_SEND_EMAIL_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqClass))

typedef struct _EasSendEmailReqClass EasSendEmailReqClass;
typedef struct _EasSendEmailReq EasSendEmailReq;
typedef struct _EasSendEmailReqPrivate EasSendEmailReqPrivate;

struct _EasSendEmailReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasSendEmailReq
{
	EasRequestBase parent_instance;

	EasSendEmailReqPrivate *priv;
};

GType eas_send_email_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new send email request GObject
 *
 * @param[in] account_uid
 *	  Unique identifier for a user account.
 * @param[in] context
 *	  DBus context token.
 * @param client_id  
 *      The client ID identifying the email on the local system.
 * @param[in] mime_file
 *	  Full path to local file system directory where the retrieved email.
 *
 * @return An allocated EasSendEmailReq GObject or NULL
 */
EasSendEmailReq *
eas_send_email_req_new(const gchar* account_id, 
                       DBusGMethodInvocation *context, 
                       const gchar* client_id, 
                       const gchar* mime_file);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasSendEmailReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean 
eas_send_email_req_Activate(EasSendEmailReq *self, 
                            GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then returning the results across the dbus to the client 
 *
 * @param[in] self
 *	  The EasSendEmailReq GObject instance whose messages are complete.
 * @param[in] doc
 *    Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if finished and needs unreffing, FALSE otherwise.
 */
gboolean 
eas_send_email_req_MessageComplete(EasSendEmailReq *self, 
                                   xmlDoc* doc, 
                                   GError* error);


G_END_DECLS

#endif /* _EAS_SEND_EMAIL_REQ_H_ */
