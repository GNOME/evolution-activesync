/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_GET_EMAIL_ATTACHMENT_MSG_H_
#define _EAS_GET_EMAIL_ATTACHMENT_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG             (eas_get_email_attachment_msg_get_type ())
#define EAS_GET_EMAIL_ATTACHMENT_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsg))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_IS_GET_EMAIL_ATTACHMENT_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG))
#define EAS_GET_EMAIL_ATTACHMENT_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_ATTACHMENT_MSG, EasGetEmailAttachmentMsgClass))

typedef struct _EasGetEmailAttachmentMsgClass EasGetEmailAttachmentMsgClass;
typedef struct _EasGetEmailAttachmentMsg EasGetEmailAttachmentMsg;
typedef struct _EasGetEmailAttachmentMsgPrivate EasGetEmailAttachmentMsgPrivate;

struct _EasGetEmailAttachmentMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailAttachmentMsg
{
	EasMsgBase parent_instance;
	
	EasGetEmailAttachmentMsgPrivate* priv;
};

GType eas_get_email_attachment_msg_get_type (void) G_GNUC_CONST;

/**
 * Create a new email attachment message.
 *
 * @param[in] fileReference
 *	  Reference used to identify the file on the server. 
 * @param[in] directoryPath
 *	  Full path to the directory the file will be written to on the local file system.
 *
 * @return NULL or a newly created EasGetEmailAttachmentMsg GObject.
 */
EasGetEmailAttachmentMsg* 
eas_get_email_attachment_msg_new (const gchar *fileReference, 
								  const gchar* directoryPath);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasGetEmailAttachmentMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* 
eas_get_email_attachment_msg_build_message (EasGetEmailAttachmentMsg* self);


/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasGetEmailAttachmenentMsg GObject instance 
 * was created.
 *
 * @param[in] self
 *	  The EasGetEmailAttachmentMsg GObject instance.
 * @param[in] doc
 *	  libxml DOM tree structure containing the XML to be parsed. [no transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean 
eas_get_email_attachment_msg_parse_response (EasGetEmailAttachmentMsg* self, 
											 xmlDoc* doc, 
											 GError** error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_ATTACHMENT_MSG_H_ */
