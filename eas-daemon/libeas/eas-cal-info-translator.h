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



// Parse a response message
gchar* eas_cal_info_translator_parse_response(xmlNode* node, gchar* server_id);

G_END_DECLS

#endif /* _EAS_CAL_INFO_TRANSLATOR_H_ */
