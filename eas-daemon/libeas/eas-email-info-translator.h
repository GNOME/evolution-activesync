/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */
/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include <eas-email-info.h>

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
gchar *eas_email_info_translator_parse_add_response (const xmlNode *node, gchar *server_id);

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
gchar *eas_email_info_translator_parse_delete_response (const xmlNode *node, gchar *server_id);

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
gchar *eas_email_info_translator_parse_update_response (const xmlNode *node, gchar *server_id);


/**
 * builds the <ApplicationData> part of XML for a request using the provided EasEmailInfo
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
gboolean eas_email_info_translator_build_update_request (const xmlDocPtr doc,
							 xmlNodePtr app_data,
							 const EasEmailInfo* email_info);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
