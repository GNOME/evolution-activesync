/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_PROVISION_REQ_H_
#define _EAS_PROVISION_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_REQ             (eas_provision_req_get_type ())
#define EAS_PROVISION_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_REQ, EasProvisionReq))
#define EAS_PROVISION_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_REQ, EasProvisionReqClass))
#define EAS_IS_PROVISION_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_REQ))
#define EAS_IS_PROVISION_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_REQ))
#define EAS_PROVISION_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_REQ, EasProvisionReqClass))

typedef struct _EasProvisionReqClass EasProvisionReqClass;
typedef struct _EasProvisionReq EasProvisionReq;
typedef struct _EasProvisionReqPrivate EasProvisionReqPrivate;

struct _EasProvisionReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasProvisionReq
{
	EasRequestBase parent_instance;

	EasProvisionReqPrivate* priv;
};

GType eas_provision_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a provisioning request GObject
 *
 * @param[in] policy_status
 *	  Policy status to be set. [no transfer]
 * @param[in] policy_key
 *	  Policy key to be set. [no transfer]
 *
 * @return An allocated EasProvisionReq GObject or NULL
 */
EasProvisionReq* 
eas_provision_req_new (const gchar* policy_status, const gchar* policy_key);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasProvisionReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_provision_req_Activate (EasProvisionReq* self, GError** error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then returning the results across the dbus to the client 
 *
 * @param[in] self
 *	  The EasProvisionReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 *
 * @return TRUE if finished and needs unreffing, FALSE otherwise.
 */
gboolean eas_provision_req_MessageComplete (EasProvisionReq* self, 
                                            xmlDoc *doc, 
                                            GError* error);

G_END_DECLS

#endif /* _EAS_PROVISION_REQ_H_ */
