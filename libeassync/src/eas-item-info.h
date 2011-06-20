/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_ITEM_INFO_H_
#define _EAS_ITEM_INFO_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_ITEM_INFO             (eas_item_info_get_type ())
#define EAS_ITEM_INFO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ITEM_INFO, EasItemInfo))
#define EAS_ITEM_INFO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ITEM_INFO, EasItemInfoClass))
#define EAS_IS_ITEM_INFO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ITEM_INFO))
#define EAS_IS_ITEM_INFO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_ITEM_INFO))
#define EAS_ITEM_INFO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_ITEM_INFO, EasItemInfoClass))

typedef struct _EasItemInfoClass EasItemInfoClass;
typedef struct _EasItemInfo EasItemInfo;

struct _EasItemInfoClass
{
	GObjectClass parent_class;
};

struct _EasItemInfo
{
	GObject parent_instance;

	gchar* client_id;	   // from Local app - needed for add operations
	gchar*  server_id;		// from AS server
	gchar*  data;		// The iCalendar (RFC 5545) formatted payload
};

GType eas_item_info_get_type (void) G_GNUC_CONST;


/*
Instantiate
*/
EasItemInfo* eas_item_info_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_item_info_serialise(EasItemInfo* self, gchar** result);

/*
populate the object from a string
*/
gboolean eas_item_info_deserialise(EasItemInfo* self, const gchar* data);


G_END_DECLS

#endif /* _EAS_ITEM_INFO_H_ */
