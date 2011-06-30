#ifndef _EAS_CON_INFO_TRANSLATOR_H_
#define _EAS_CON_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>

#include "../../libeassync/src/eas-item-info.h"

/**
 * Parse a contact sync response message
 *
 * Converts the <ApplicationData> element of an Exchange ActiveSync XML response into an
 * EasItemInfo structure (containing an vCard document).
 *
 * @param app_data  
 *      <ApplicationData> element from the Exchange ActiveSync XML response
 * @param server_id  
 *      The server ID from the Exchange ActiveSync XML response
 *
 * @return Serialised EasItemInfo structure
 */
gchar* eas_con_info_translator_parse_response(xmlNodePtr app_data, 
                                              const gchar* server_id);


/**
 * Parse an EasItemInfo structure and convert to EAS XML format
 *
 * @param appData
 *      The top-level <ApplicationData> XML element in which to store all the parsed elements
 * @param calInfo
 *      The EasItemInfo struct containing the vCard string to parse (plus a server ID)
 */
gboolean eas_con_info_translator_parse_request(xmlDocPtr doc, 
                                               xmlNodePtr app_data, 
                                               EasItemInfo* cal_info);

G_END_DECLS

#endif /* _EAS_CON_INFO_TRANSLATOR_H_ */
