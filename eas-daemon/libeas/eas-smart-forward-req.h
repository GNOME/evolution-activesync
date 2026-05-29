#ifndef _EAS_SMART_FORWARD_REQ_H_
#define _EAS_SMART_FORWARD_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SMART_FORWARD_REQ             (eas_smart_forward_req_get_type ())
#define EAS_SMART_FORWARD_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SMART_FORWARD_REQ, EasSmartForwardReq))
#define EAS_SMART_FORWARD_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SMART_FORWARD_REQ, EasSmartForwardReqClass))
#define EAS_IS_SMART_FORWARD_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SMART_FORWARD_REQ))
#define EAS_IS_SMART_FORWARD_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SMART_FORWARD_REQ))
#define EAS_SMART_FORWARD_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SMART_FORWARD_REQ, EasSmartForwardReqClass))

typedef struct _EasSmartForwardReqClass EasSmartForwardReqClass;
typedef struct _EasSmartForwardReq EasSmartForwardReq;
typedef struct _EasSmartForwardReqPrivate EasSmartForwardReqPrivate;

struct _EasSmartForwardReqClass {
	EasRequestBaseClass parent_class;
};

struct _EasSmartForwardReq {
	EasRequestBase parent_instance;
	EasSmartForwardReqPrivate *priv;
};

GType eas_smart_forward_req_get_type (void) G_GNUC_CONST;

EasSmartForwardReq *eas_smart_forward_req_new (const gchar *account_id,
					       GDBusMethodInvocation *context,
					       const gchar *client_id,
					       const gchar *source_folder_id,
					       const gchar *source_item_id,
					       const gchar *mime_file);

gboolean eas_smart_forward_req_Activate (EasSmartForwardReq *self, GError **error);

gboolean eas_smart_forward_req_MessageComplete (EasSmartForwardReq *self,
						xmlDoc *doc,
						GError *error);

G_END_DECLS

#endif /* _EAS_SMART_FORWARD_REQ_H_ */
