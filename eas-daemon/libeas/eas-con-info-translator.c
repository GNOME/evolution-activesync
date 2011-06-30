#include "eas-cal-info-translator.h"

#include <evolution-data-server-2.32/libebook/e-vcard.h>


// EAS string value definitions
#define EAS_ELEMENT_APPLICATIONDATA           "ApplicationData"
#define EAS_ELEMENT_FIRSTNAME                 "FirstName"
#define EAS_ELEMENT_MIDDLENAME                "MiddleName"
#define EAS_ELEMENT_LASTNAME                  "LastName"
#define EAS_ELEMENT_TITLE                     "Title"
#define EAS_ELEMENT_SUFFIX                    "Suffix"

void add_attr_value(EVCardAttribute *attr,xmlNodePtr *node,const gchar *sought)
{
	xmlNodePtr n = node;
	gchar *value = NULL; 
	g_debug("jba1");
	g_debug(sought);

	// find sought value 
	while (n) 
	{
	g_debug((const gchar*)(n->name));
		if (!xmlStrcmp((const gchar*)(n->name), (const xmlChar *)sought))
		{
	g_debug("jba2");
			value = xmlNodeGetContent(n);
			break;
		}
		n = n->next;
	}
	// drop out if not found
	if(!value)
		return;
	// add sought value
	g_debug("jba3");
	e_vcard_attribute_add_value(attr, value);
}

void add_name_attr_values(EVCardAttribute *attr,xmlNodePtr *node)
{
	add_attr_value(attr, node, EAS_ELEMENT_FIRSTNAME);
	add_attr_value(attr, node, EAS_ELEMENT_MIDDLENAME);
	add_attr_value(attr, node, EAS_ELEMENT_LASTNAME);
	add_attr_value(attr, node, EAS_ELEMENT_TITLE);
	add_attr_value(attr, node, EAS_ELEMENT_SUFFIX);
}

                    
gchar* eas_con_info_translator_parse_response(xmlNodePtr node, 
                                              const gchar* server_id)
{
	// Variable for the return value
	gchar* result = NULL;
	EVCard *vcard;
	g_debug("eas_con_info_translator_parse_response ++");


	// Variable for property values as they're read from the XML
//	gchar* value  = NULL;

	if (node && (node->type == XML_ELEMENT_NODE) && (!g_strcmp0((char*)(node->name), EAS_ELEMENT_APPLICATIONDATA)))
	{
		xmlNodePtr n = node;
//		VObject *prop;
		
//		EasItemInfo* conInfo = NULL;

		EVCardAttribute *Nattr = NULL;

		// vCard object
		vcard = e_vcard_new();
//		nameProp = addProp(vcard,VCNameProp);

		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);

				//
				// First Name
				//				
				if ((g_strcmp0(name, EAS_ELEMENT_FIRSTNAME) == 0) && (Nattr == NULL))
				{
					Nattr = e_vcard_attribute_new(NULL, EVC_N);
					add_name_attr_values(Nattr, node->children);
					e_vcard_add_attribute(vcard, Nattr);
				}
			}			
		}		
		// Now insert the server ID and vCard into an EasItemInfo object and serialise it

	}

	result = e_vcard_to_string(vcard, EVC_FORMAT_VCARD_30);

	g_debug(result);
	g_debug("eas_con_info_translator_parse_response --");
	return result;	
}


gboolean eas_con_info_translator_parse_request(xmlDocPtr doc, 
                                               xmlNodePtr app_data, 
                                               EasItemInfo* cal_info)
{
	gboolean success = FALSE;

	return success;
}