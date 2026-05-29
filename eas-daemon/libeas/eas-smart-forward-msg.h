#ifndef _EAS_SMART_FORWARD_MSG_H_
#define _EAS_SMART_FORWARD_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SMART_FORWARD_MSG             (eas_smart_forward_msg_get_type ())
#define EAS_SMART_FORWARD_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SMART_FORWARD_MSG, EasSmartForwardMsg))
#define EAS_SMART_FORWARD_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SMART_FORWARD_MSG, EasSmartForwardMsgClass))
#define EAS_IS_SMART_FORWARD_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SMART_FORWARD_MSG))
#define EAS_IS_SMART_FORWARD_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SMART_FORWARD_MSG))
#define EAS_SMART_FORWARD_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SMART_FORWARD_MSG, EasSmartForwardMsgClass))

typedef struct _EasSmartForwardMsgClass EasSmartForwardMsgClass;
typedef struct _EasSmartForwardMsg EasSmartForwardMsg;
typedef struct _EasSmartForwardMsgPrivate EasSmartForwardMsgPrivate;

struct _EasSmartForwardMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasSmartForwardMsg {
	EasMsgBase parent_instance;
	EasSmartForwardMsgPrivate *priv;
};

GType eas_smart_forward_msg_get_type (void) G_GNUC_CONST;

EasSmartForwardMsg *eas_smart_forward_msg_new (const gchar *account_id,
					       const gchar *client_id,
					       const gchar *source_folder_id,
					       const gchar *source_item_id,
					       gchar *mime_string);

xmlDoc *eas_smart_forward_msg_build_message (EasSmartForwardMsg *self);

gboolean eas_smart_forward_msg_parse_response (EasSmartForwardMsg *self,
					       xmlDoc *doc,
					       GError **error);

G_END_DECLS

#endif /* _EAS_SMART_FORWARD_MSG_H_ */
