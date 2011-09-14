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

#ifndef _EAS_CAL_INFO_TRANSLATOR_H_
#define _EAS_CAL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>

#include <eas-item-info.h>

/**
 * Parse a calendar sync response message
 *
 * Converts the <ApplicationData> element of an Exchange ActiveSync XML response into an
 * EasCalInfo structure (containing an iCalendar document).
 *
 * @param app_data
 *      <ApplicationData> element from the Exchange ActiveSync XML response
 * @param[in] server_id [full transfer]
 *      The server ID from the Exchange ActiveSync XML response, will be freed by g_free()
 *
 * @return Serialised EasCalInfo structure
 */
gchar* eas_cal_info_translator_parse_response (xmlNodePtr app_data,
					       gchar* server_id);


/**
 * Parse an EasCalInfo structure and convert to EAS XML format
 *
 * @param  doc
 *      REDUNDANT PARAMETER: only required for debug output. TODO: remove this
 * @param appData
 *      The top-level <ApplicationData> XML element in which to store all the parsed elements
 * @param calInfo
 *      The EasCalInfo struct containing the iCalendar string to parse (plus a server ID)
 */
gboolean eas_cal_info_translator_parse_request (xmlDocPtr doc,
						xmlNodePtr app_data,
						EasItemInfo* cal_info);

G_END_DECLS

#endif /* _EAS_CAL_INFO_TRANSLATOR_H_ */
