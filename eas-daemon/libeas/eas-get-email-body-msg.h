/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_GET_EMAIL_BODY_MSG_H_
#define _EAS_GET_EMAIL_BODY_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_GET_EMAIL_BODY_MSG             (eas_get_email_body_msg_get_type ())
#define EAS_GET_EMAIL_BODY_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsg))
#define EAS_GET_EMAIL_BODY_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))
#define EAS_IS_GET_EMAIL_BODY_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_IS_GET_EMAIL_BODY_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_GET_EMAIL_BODY_MSG))
#define EAS_GET_EMAIL_BODY_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_GET_EMAIL_BODY_MSG, EasGetEmailBodyMsgClass))

typedef struct _EasGetEmailBodyMsgClass EasGetEmailBodyMsgClass;
typedef struct _EasGetEmailBodyMsg EasGetEmailBodyMsg;
typedef struct _EasGetEmailBodyMsgPrivate EasGetEmailBodyMsgPrivate;

struct _EasGetEmailBodyMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasGetEmailBodyMsg
{
	EasMsgBase parent_instance;

	EasGetEmailBodyMsgPrivate* priv;
};

GType eas_get_email_body_msg_get_type (void) G_GNUC_CONST;


/**
 * Create a new email body message.
 *
 * @param[in] accountUid
 *	  Unique identifier for a user account.
 * @param[in] collectionId
 *	  The identifer for the target server folder.
 * @param[in] directoryPath
 *	  Full path to the directory the file will be written to on the local file system.
 *
 * @return NULL or a newly created EasGetEmailAttachmentMsg GObject.
 */
EasGetEmailBodyMsg* 
eas_get_email_body_msg_new (const gchar* serverUid, 
							const gchar* collectionId, 
							const gchar* directoryPath);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasGetEmailBodyMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* 
eas_get_email_body_msg_build_message (EasGetEmailBodyMsg* self);

/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasGetEmailBodyMsg GObject instance was 
 * created.
 *
 * @param[in] self
 *	  The EasGetEmailBodyMsg GObject instance.
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
eas_get_email_body_msg_parse_response (EasGetEmailBodyMsg* self, 
									   xmlDoc* doc, 
									   GError** error);

G_END_DECLS

#endif /* _EAS_GET_EMAIL_BODY_MSG_H_ */
