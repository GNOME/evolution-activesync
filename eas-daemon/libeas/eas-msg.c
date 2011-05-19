#include "eas-msg.h"

xmlDoc*
build_provision_as_xml(const gchar* policy_key, const gchar* status)
{
    xmlDoc *doc = NULL;
    xmlDtd *dtd = NULL;
    xmlNode *node = NULL, *child = NULL, *grandchild = NULL;
    xmlNs *ns = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode(doc, NULL, (xmlChar*)"Provision", NULL);
    xmlDocSetRootElement (doc, node);

    dtd = xmlCreateIntSubset(doc, 
                    (xmlChar*)"ActiveSync", 
                    (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                    (xmlChar*)"http://www.microsoft.com/");
                    
    ns = xmlNewNs (node, (xmlChar *)"Provision:",NULL);
    xmlNewNsProp(node, ns, (xmlChar*)"xmlns:settings", (xmlChar*)"Settings:");
    child = xmlNewChild(node, ns, (xmlChar *)"Policies", NULL);
    grandchild = xmlNewChild(child, ns, (xmlChar*)"Policy", NULL);
    xmlNewChild(grandchild, ns, (xmlChar*)"PolicyType",(xmlChar*)"MS-EAS-Provisioning-WBXML");
    if (policy_key) {
        xmlNewChild(grandchild, ns, (xmlChar*)"PolicyKey",(xmlChar*)policy_key);
    }
    if (status) {
        xmlNewChild(grandchild, ns, (xmlChar*)"Status",(xmlChar*)status);
    }

    return doc;
}

xmlDoc *
build_folder_sync_as_xml(const gchar* syncKey)
{
    xmlDoc *doc = NULL;
    xmlNode *node = NULL, *child = NULL;
    xmlNs *ns = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar*)"FolderSync", NULL);
    xmlDocSetRootElement (doc, node);
    
    xmlCreateIntSubset(doc, 
                       (xmlChar*)"ActiveSync", 
                       (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                       (xmlChar*)"http://www.microsoft.com/");

    ns = xmlNewNs (node, (xmlChar *)"FolderHierarchy:",NULL);
    child = xmlNewChild(node, NULL, (xmlChar *)"SyncKey", (xmlChar*)syncKey);

    return doc;
}
