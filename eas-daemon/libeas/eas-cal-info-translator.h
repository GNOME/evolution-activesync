/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#ifndef _EAS_CAL_INFO_TRANSLATOR_H_
#define _EAS_CAL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>


G_BEGIN_DECLS

#define EAS_TYPE_CAL_INFO_TRANSLATOR             (eas_cal_info_translator_get_type ())
#define EAS_CAL_INFO_TRANSLATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CAL_INFO_TRANSLATOR, EasCalInfoTranslator))
#define EAS_CAL_INFO_TRANSLATOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CAL_INFO_TRANSLATOR, EasCalInfoTranslatorClass))
#define EAS_IS_CAL_INFO_TRANSLATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CAL_INFO_TRANSLATOR))
#define EAS_IS_CAL_INFO_TRANSLATOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CAL_INFO_TRANSLATOR))
#define EAS_CAL_INFO_TRANSLATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CAL_INFO_TRANSLATOR, EasCalInfoTranslatorClass))

typedef struct _EasCalInfoTranslatorClass EasCalInfoTranslatorClass;
typedef struct _EasCalInfoTranslator EasCalInfoTranslator;

struct _EasCalInfoTranslatorClass
{
	GObjectClass parent_class;
};

struct _EasCalInfoTranslator
{
	GObject parent_instance;
};

GType eas_cal_info_translator_get_type (void) G_GNUC_CONST;

// Constructor
EasCalInfoTranslator* eas_cal_info_translator_new();

// Parse a response message
gchar* eas_cal_info_translator_parse_response(EasCalInfoTranslator* self, xmlNode* node, gchar* server_id);

G_END_DECLS

#endif /* _EAS_CAL_INFO_TRANSLATOR_H_ */
