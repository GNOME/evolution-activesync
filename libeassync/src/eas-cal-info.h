/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_CAL_INFO_H_
#define _EAS_CAL_INFO_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_CAL_INFO             (eas_cal_info_get_type ())
#define EAS_CAL_INFO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CAL_INFO, EasCalInfo))
#define EAS_CAL_INFO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CAL_INFO, EasCalInfoClass))
#define EAS_IS_CAL_INFO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CAL_INFO))
#define EAS_IS_CAL_INFO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CAL_INFO))
#define EAS_CAL_INFO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CAL_INFO, EasCalInfoClass))

typedef struct _EasCalInfoClass EasCalInfoClass;
typedef struct _EasCalInfo EasCalInfo;

struct _EasCalInfoClass
{
	GObjectClass parent_class;
};

struct _EasCalInfo
{
	GObject parent_instance;

	gchar* client_id;	   // from Local app - needed for add operations
	gchar*  server_id;		// from AS server
	gchar*  icalendar;		// The iCalendar (RFC 5545) formatted payload
};

GType eas_cal_info_get_type (void) G_GNUC_CONST;


/*
Instantiate
*/
EasCalInfo* eas_cal_info_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_cal_info_serialise(EasCalInfo* self, gchar** result);

/*
populate the object from a string
*/
gboolean eas_cal_info_deserialise(EasCalInfo* self, const gchar* data);


G_END_DECLS

#endif /* _EAS_CAL_INFO_H_ */
