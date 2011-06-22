#ifndef _EAS_CAL_INFO_TRANSLATOR_H_
#define _EAS_CAL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>

#include "../../libeassync/src/eas-item-info.h"

/**
 * Parse a calendar sync response message
 *
 * Converts the <ApplicationData> element of an Exchange ActiveSync XML response into an
 * EasCalInfo structure (containing an iCalendar document).
 *
 * @param app_data  
 *      <ApplicationData> element from the Exchange ActiveSync XML response
 * @param server_id  
 *      The server ID from the Exchange ActiveSync XML response
 *
 * @return Serialised EasCalInfo structure
 */
gchar* eas_cal_info_translator_parse_response(xmlNodePtr app_data, 
                                              const gchar* server_id);


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
gboolean eas_cal_info_translator_parse_request(xmlDocPtr doc, 
                                               xmlNodePtr app_data, 
                                               EasItemInfo* cal_info);

G_END_DECLS

#endif /* _EAS_CAL_INFO_TRANSLATOR_H_ */
