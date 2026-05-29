#ifndef _EAS_SMART_REPLY_REQ_H_
#define _EAS_SMART_REPLY_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SMART_REPLY_REQ             (eas_smart_reply_req_get_type ())
#define EAS_SMART_REPLY_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SMART_REPLY_REQ, EasSmartReplyReq))
#define EAS_SMART_REPLY_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SMART_REPLY_REQ, EasSmartReplyReqClass))
#define EAS_IS_SMART_REPLY_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SMART_REPLY_REQ))
#define EAS_IS_SMART_REPLY_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SMART_REPLY_REQ))
#define EAS_SMART_REPLY_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SMART_REPLY_REQ, EasSmartReplyReqClass))

typedef struct _EasSmartReplyReqClass EasSmartReplyReqClass;
typedef struct _EasSmartReplyReq EasSmartReplyReq;
typedef struct _EasSmartReplyReqPrivate EasSmartReplyReqPrivate;

struct _EasSmartReplyReqClass {
	EasRequestBaseClass parent_class;
};

struct _EasSmartReplyReq {
	EasRequestBase parent_instance;
	EasSmartReplyReqPrivate *priv;
};

GType eas_smart_reply_req_get_type (void) G_GNUC_CONST;

EasSmartReplyReq *eas_smart_reply_req_new (const gchar *account_id,
					   GDBusMethodInvocation *context,
					   const gchar *client_id,
					   const gchar *source_folder_id,
					   const gchar *source_item_id,
					   const gchar *mime_file);

gboolean eas_smart_reply_req_Activate (EasSmartReplyReq *self, GError **error);

gboolean eas_smart_reply_req_MessageComplete (EasSmartReplyReq *self,
					      xmlDoc *doc,
					      GError *error);

G_END_DECLS

#endif /* _EAS_SMART_REPLY_REQ_H_ */
