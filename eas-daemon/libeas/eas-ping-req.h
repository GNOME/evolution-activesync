
#ifndef _EAS_PING_REQ_H_
#define _EAS_PING_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_PING_REQ             (eas_ping_req_get_type ())
#define EAS_PING_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PING_REQ, EasPingReq))
#define EAS_PING_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PING_REQ, EasPingReqClass))
#define EAS_IS_PING_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PING_REQ))
#define EAS_IS_PING_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PING_REQ))
#define EAS_PING_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PING_REQ, EasPingReqClass))

typedef struct _EasPingReqClass EasPingReqClass;
typedef struct _EasPingReq EasPingReq;
typedef struct _EasPingReqPrivate EasPingReqPrivate;

typedef enum
{
    EasPingReqSend = 0,
    EasPingReqSendHeartbeat,
	EasPingReqNotifyClient,
	EasPingReqHeartbeatError,
	EasPingReqFolderError
} EasPingReqState;

struct _EasPingReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasPingReq
{
	EasRequestBase parent_instance;

	EasPingReqPrivate * priv;
};

GType eas_ping_req_get_type (void) G_GNUC_CONST;

// C'tor
EasPingReq *eas_ping_req_new (const gchar* account_id, const gchar *heartbeat, const GSList* folder_list, DBusGMethodInvocation *context);

// start async request
gboolean eas_ping_req_Activate(EasPingReq *self, GError** error);

// async request completed
gboolean eas_ping_req_MessageComplete(EasPingReq *self, xmlDoc* doc, GError* error);


G_END_DECLS

#endif /* _EAS_PING_REQ_H_ */
