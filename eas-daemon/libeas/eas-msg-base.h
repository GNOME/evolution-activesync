/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_MSG_BASE_H_
#define _EAS_MSG_BASE_H_

#include <glib-object.h>
#include <libxml/xmlreader.h> // xmlDoc

G_BEGIN_DECLS

#define EAS_TYPE_MSG_BASE             (eas_msg_base_get_type ())
#define EAS_MSG_BASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_MSG_BASE, EasMsgBase))
#define EAS_MSG_BASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_MSG_BASE, EasMsgBaseClass))
#define EAS_IS_MSG_BASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_MSG_BASE))
#define EAS_IS_MSG_BASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_MSG_BASE))
#define EAS_MSG_BASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_MSG_BASE, EasMsgBaseClass))

typedef struct _EasMsgBaseClass EasMsgBaseClass;
typedef struct _EasMsgBase EasMsgBase;
typedef struct _EasMsgBasePrivate EasMsgBasePrivate;


struct _EasMsgBaseClass
{
	GObjectClass parent_class;
};

struct _EasMsgBase
{
	GObject parent_instance;

	EasMsgBasePrivate *priv;
};

typedef enum {
	EAS_MSG_BASE = 0, // Not actually valid.
	EAS_SYNC_FOLDER_HIERARCHY,
	EAS_FOLDER_SYNC,

	// Etc
	
	EAS_LAST
} EasMsgType;

GType eas_msg_base_get_type (void) G_GNUC_CONST;

xmlDoc* eas_msg_base_build_message (EasMsgBase *self);
void eas_msg_base_parse_response (EasMsgBase *self, xmlDoc* doc);

G_END_DECLS

#endif /* _EAS_MSG_BASE_H_ */
