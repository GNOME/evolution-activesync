#ifndef _EAS_FIND_REQ_H_
#define _EAS_FIND_REQ_H_

#include <glib-object.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_FIND_REQ             (eas_find_req_get_type ())
#define EAS_FIND_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_FIND_REQ, EasFindReq))
#define EAS_FIND_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_FIND_REQ, EasFindReqClass))
#define EAS_IS_FIND_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_FIND_REQ))
#define EAS_IS_FIND_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_FIND_REQ))
#define EAS_FIND_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_FIND_REQ, EasFindReqClass))

typedef struct _EasFindReqClass EasFindReqClass;
typedef struct _EasFindReq EasFindReq;
typedef struct _EasFindReqPrivate EasFindReqPrivate;

struct _EasFindReqClass {
	EasRequestBaseClass parent_class;
};

struct _EasFindReq {
	EasRequestBase parent_instance;
	EasFindReqPrivate *priv;
};

GType eas_find_req_get_type (void) G_GNUC_CONST;

EasFindReq *eas_find_req_new (const gchar *account_id,
			      GDBusMethodInvocation *context,
			      const gchar *folder_id,
			      const gchar *query,
			      guint range_start,
			      guint range_end);

gboolean eas_find_req_Activate (EasFindReq *self, GError **error);

gboolean eas_find_req_MessageComplete (EasFindReq *self,
				       xmlDoc *doc,
				       GError *error);

G_END_DECLS

#endif /* _EAS_FIND_REQ_H_ */
