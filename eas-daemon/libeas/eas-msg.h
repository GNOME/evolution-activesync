#ifndef EAS_MSG_H
#define EAS_MSG_H

#include <glib-object.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

xmlDoc* build_provision_as_xml(const gchar* policy_key, const gchar* status);
xmlDoc* build_folder_sync_as_xml(const gchar* syncKey);

G_END_DECLS
#endif /* EAS_MSG_H */
