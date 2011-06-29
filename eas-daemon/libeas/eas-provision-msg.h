/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_PROVISION_MSG_H_
#define _EAS_PROVISION_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_MSG             (eas_provision_msg_get_type ())
#define EAS_PROVISION_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsg))
#define EAS_PROVISION_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))
#define EAS_IS_PROVISION_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_MSG))
#define EAS_IS_PROVISION_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_MSG))
#define EAS_PROVISION_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_MSG, EasProvisionMsgClass))

typedef struct _EasProvisionMsgClass EasProvisionMsgClass;
typedef struct _EasProvisionMsg EasProvisionMsg;
typedef struct _EasProvisionMsgPrivate EasProvisionMsgPrivate;

struct _EasProvisionMsgClass
{
	EasMsgBaseClass parent_class;
};

struct _EasProvisionMsg
{
	EasMsgBase parent_instance;

	EasProvisionMsgPrivate *priv;
};

GType eas_provision_msg_get_type (void) G_GNUC_CONST;


/**
 * Create a new provisioning message.
 *
 * @return NULL or a newly created EasProvisionMsg GObject.
 */
EasProvisionMsg* eas_provision_msg_new (void);

/**
 * Build the XML required for the message to be send in the request to the server.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or libxml DOM tree structure containing the XML for the message 
 *		   body. Caller is responsible for freeing the result using xmlFreeDoc().
 *		   [full transfer]
 */
xmlDoc* eas_provision_msg_build_message (EasProvisionMsg* self);

/**
 * Parses the response from the server, storing the email attachment according 
 * to the parameters set when the EasProvisionMsg GObject instance was 
 * created.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
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
eas_provision_msg_parse_response (EasProvisionMsg* self, 
                                  xmlDoc* doc, 
                                  GError** error);

/**
 * Getter for policy key.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or the policy_key. [no transfer]
 */
gchar* eas_provision_msg_get_policy_key (EasProvisionMsg* self);

/**
 * Getter for policy status.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 *
 * @return NULL or the policy_status. [no transfer]
 */
gchar* eas_provision_msg_get_policy_status (EasProvisionMsg* self);


/**
 * Setter for policy key.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 * @param[in] policyKey
 *	  New policy key. [no transfer]
 */
void eas_provision_msg_set_policy_key (EasProvisionMsg* self, const gchar* policyKey);

/**
 * Setter for policy status.
 *
 * @param[in] self
 *	  The EasProvisionMsg GObject instance.
 * @param[in] policyStatus
 *	  New policy status. [no transfer]
 */
void eas_provision_msg_set_policy_status (EasProvisionMsg* self, const gchar* policyStatus);

G_END_DECLS

#endif /* _EAS_PROVISION_MSG_H_ */
