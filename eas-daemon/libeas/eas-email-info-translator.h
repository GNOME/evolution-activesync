/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 */

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include "../../libeasmail/src/eas-email-info.h"

G_BEGIN_DECLS

// TODO rename these methods so they all start with eas_email_info_translator_

// parses the email ApplicationData for an add
gchar *eas_add_email_appdata_parse_response (xmlNode *node, gchar *server_id);

// parses the email ApplicationData for a delete
gchar *eas_delete_email_appdata_parse_response (xmlNode *node, gchar *server_id);

// parses the email ApplicationData for an update
gchar *eas_update_email_appdata_parse_response ( xmlNode *node, gchar *server_id);

// parse a request. Populate the ApplicationData node with details from the email_info object
gboolean eas_email_info_translator_parse_request(xmlDocPtr doc, xmlNodePtr app_data, EasEmailInfo* email_info);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
