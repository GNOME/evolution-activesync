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

#include "../../libeascal/src/eas-cal-info.h"



// Parse a response message
// TODO: this should return a gboolean too
gchar* eas_cal_info_translator_parse_response(xmlNodePtr app_data, const gchar* server_id);

// Parse a request message
gboolean eas_cal_info_translator_parse_request(xmlDocPtr doc, xmlNodePtr app_data, EasCalInfo* cal_info);

G_END_DECLS

#endif /* _EAS_CAL_INFO_TRANSLATOR_H_ */
