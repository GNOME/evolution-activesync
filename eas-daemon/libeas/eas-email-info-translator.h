/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include "../../libeasmail/src/eas-email-info.h"

G_BEGIN_DECLS

/**
 *
 * 
 * @param[in] node
 *	  XML 'root' node from where we will start our parsing.
 * @param[in] server_id [full transfer]
 *	  Server identifier for this email, will be freed by g_free()
 *
 * @result NULL or A serialized EasEmailInfo GObject, caller is responsible 
 * for freeing the data with g_free().
 */
gchar *eas_email_info_translator_add (const xmlNode *node, gchar *server_id);

/**
 *
 * 
 * @param[in] node
 *	  XML 'root' node from where we will start our parsing.
 * @param[in] server_id [full transfer]
 *	  Server identifier for this email, will be freed by g_free()
 *
 * @result NULL or A serialized EasEmailInfo GObject, caller is responsible 
 * for freeing the data with g_free().
 */
gchar *eas_email_info_translator_delete (const xmlNode *node, gchar *server_id);

/**
 *
 * 
 * @param[in] node
 *	  XML 'root' node from where we will start our parsing.
 * @param[in] server_id [full transfer]
 *	  Server identifier for this email, will be freed by g_free()
 *
 * @result NULL or A serialized EasEmailInfo GObject, caller is responsible 
 * for freeing the data with g_free().
 */
gchar *eas_email_info_translator_update (const xmlNode *node, gchar *server_id);


/**
 *
 * 
 * @param[in] doc
 *	  ONLY USED FOR DEBUG
 * @param app_data
 *	  Existing XML node point at which we need to insert the read flag 
 *	  status and catagory list.
 * @param[in] email_info
 *	  Source email structure from which we are extracting the data.
 *
 * @result TRUE if successful, otherwise FALSE.
 */
gboolean eas_email_info_translator_build_update_request(const xmlDocPtr doc, 
                                                        xmlNodePtr app_data, 
                                                        const EasEmailInfo* email_info);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
