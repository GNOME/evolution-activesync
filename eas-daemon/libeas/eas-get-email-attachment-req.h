/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_GET_EMAIL_ATTACHMENT_REQ_H_
#define _EAS_GET_EMAIL_ATTACHMENT_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"
G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ             (eas_get_email_attachment_req_get_type ())
#define EAS_GET_EMAIL_ATTACHMENT_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReq))
#define EAS_GET_EMAIL_ATTACHMENT_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqClass))
#define EAS_IS_GET_EMAIL_ATTACHMENT_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ))
#define EAS_IS_GET_EMAIL_ATTACHMENT_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ))
#define EAS_GET_EMAIL_ATTACHMENT_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_REQ, EasGetEmailAttachmentReqClass))

typedef struct _EasGetEmailAttachmentReqClass EasGetEmailAttachmentReqClass;
typedef struct _EasGetEmailAttachmentReq EasGetEmailAttachmentReq;
typedef struct _EasGetEmailAttachmentReqPrivate EasGetEmailAttachmentReqPrivate;

struct _EasGetEmailAttachmentReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasGetEmailAttachmentReq
{
	EasRequestBase parent_instance;
	
	EasGetEmailAttachmentReqPrivate* priv;
};

GType eas_get_email_attachment_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new delete email request GObject
 *
 * @param[in] account_id
 *	  Unique identifier for a user account.
 * @param[in] file_reference
 *	  File identifer reference on the server.
 * @param[in] mime_directory
 *	  Full path to local file system directory where the retrieved email 
 *	  attachment is to be written.
 * @param[in] context
 *	  DBus context token.
 *
 * @return An allocated EasGetEmailAttachmentReq GObject or NULL
 */
EasGetEmailAttachmentReq* 
eas_get_email_attachment_req_new (const gchar* account_uid, 
                                  const gchar *file_reference,
                                  const gchar *mime_directory,
                                  DBusGMethodInvocation *context);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasGetEmailAttachmentReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean 
eas_get_email_attachment_req_Activate (EasGetEmailAttachmentReq* self, 
                                       GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then returning the results across the dbus to the client *
 * @param[in] self
 *	  The EasGetEmailAttachmentReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 * @return TRUE if finished and needs unreffing, FALSE otherwise
 */
gboolean 
eas_get_email_attachment_req_MessageComplete (EasGetEmailAttachmentReq* self, 
                                              xmlDoc *doc, 
                                              GError* error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_ATTACHMENT_REQ_H_ */
