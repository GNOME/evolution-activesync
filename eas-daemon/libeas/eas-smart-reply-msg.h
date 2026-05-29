#ifndef _EAS_SMART_REPLY_MSG_H_
#define _EAS_SMART_REPLY_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_SMART_REPLY_MSG             (eas_smart_reply_msg_get_type ())
#define EAS_SMART_REPLY_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SMART_REPLY_MSG, EasSmartReplyMsg))
#define EAS_SMART_REPLY_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SMART_REPLY_MSG, EasSmartReplyMsgClass))
#define EAS_IS_SMART_REPLY_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SMART_REPLY_MSG))
#define EAS_IS_SMART_REPLY_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SMART_REPLY_MSG))
#define EAS_SMART_REPLY_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SMART_REPLY_MSG, EasSmartReplyMsgClass))

typedef struct _EasSmartReplyMsgClass EasSmartReplyMsgClass;
typedef struct _EasSmartReplyMsg EasSmartReplyMsg;
typedef struct _EasSmartReplyMsgPrivate EasSmartReplyMsgPrivate;

struct _EasSmartReplyMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasSmartReplyMsg {
	EasMsgBase parent_instance;
	EasSmartReplyMsgPrivate *priv;
};

GType eas_smart_reply_msg_get_type (void) G_GNUC_CONST;

EasSmartReplyMsg *eas_smart_reply_msg_new (const gchar *account_id,
					   const gchar *client_id,
					   const gchar *source_folder_id,
					   const gchar *source_item_id,
					   gchar *mime_string);

xmlDoc *eas_smart_reply_msg_build_message (EasSmartReplyMsg *self);

gboolean eas_smart_reply_msg_parse_response (EasSmartReplyMsg *self,
					     xmlDoc *doc,
					     GError **error);

G_END_DECLS

#endif /* _EAS_SMART_REPLY_MSG_H_ */
