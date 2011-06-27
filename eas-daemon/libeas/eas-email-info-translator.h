/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include "../../libeasmail/src/eas-email-info.h"

G_BEGIN_DECLS

// TODO Get Lora to write the commment block for these functions

/**
 *
 * 
 * @param node
 *	  XML 'root' node from where we will start our parsing.
 * @param server_id
 *	  ???
 *
 * @result NULL or ???
 */
gchar *eas_email_info_translator_add (xmlNode *node, gchar *server_id);

/**
 *
 * 
 * @param node
 *	  XML 'root' node from where we will start our parsing.
 * @param server_id
 *	  ???
 *
 * @result NULL or ???
 */
gchar *eas_email_info_translator_delete (xmlNode *node, gchar *server_id);

/**
 *
 * 
 * @param node
 *	  XML 'root' node from where we will start our parsing.
 * @param server_id
 *	  ???
 *
 * @result NULL or ???
 */
gchar *eas_email_info_translator_update ( xmlNode *node, gchar *server_id);


/**
 *
 * 
 * @param doc
 *	  ???
 * @param app_data
 *	  ???
 * @param email_info
 *	  ???
 *
 * @result TRUE if successful, otherwise FALSE.
 */
gboolean eas_email_info_translator_build_update_request(xmlDocPtr doc, 
                                                        xmlNodePtr app_data, 
                                                        const EasEmailInfo* email_info);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
