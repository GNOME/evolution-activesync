/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 */

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

// parses the email ApplicationData for an add
gchar *eas_add_email_appdata_parse_response (xmlNode *node, gchar *server_id);

// parses the email ApplicationData for a delete
gchar *eas_delete_email_appdata_parse_response (xmlNode *node, gchar *server_id);

// parses the email ApplicationData for an update
gchar *eas_update_email_appdata_parse_response ( xmlNode *node, gchar *server_id);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
