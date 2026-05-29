#ifndef _EAS_FIND_MSG_H_
#define _EAS_FIND_MSG_H_

#include <glib-object.h>
#include "eas-msg-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_FIND_MSG             (eas_find_msg_get_type ())
#define EAS_FIND_MSG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_FIND_MSG, EasFindMsg))
#define EAS_FIND_MSG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_FIND_MSG, EasFindMsgClass))
#define EAS_IS_FIND_MSG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_FIND_MSG))
#define EAS_IS_FIND_MSG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_FIND_MSG))
#define EAS_FIND_MSG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_FIND_MSG, EasFindMsgClass))

typedef struct _EasFindMsgClass EasFindMsgClass;
typedef struct _EasFindMsg EasFindMsg;
typedef struct _EasFindMsgPrivate EasFindMsgPrivate;

struct _EasFindMsgClass {
	EasMsgBaseClass parent_class;
};

struct _EasFindMsg {
	EasMsgBase parent_instance;
	EasFindMsgPrivate *priv;
};

GType eas_find_msg_get_type (void) G_GNUC_CONST;

EasFindMsg *eas_find_msg_new (const gchar *folder_id,
			      const gchar *query_text,
			      gint range_start,
			      gint range_end);

xmlDoc *eas_find_msg_build_message (EasFindMsg *self);

gboolean eas_find_msg_parse_response (EasFindMsg *self,
				      xmlDoc *doc,
				      GSList **results,
				      GError **error);

G_END_DECLS

#endif /* _EAS_FIND_MSG_H_ */
