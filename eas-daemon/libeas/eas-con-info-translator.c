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

#include "eas-con-info-translator.h"

#include <libebook/e-vcard.h>
#include <string.h>

// EAS string value definitions
#define EAS_ELEMENT_APPLICATIONDATA           "ApplicationData"
#define EAS_ELEMENT_FIRSTNAME                 "FirstName"
#define EAS_ELEMENT_MIDDLENAME                "MiddleName"
#define EAS_ELEMENT_LASTNAME                  "LastName"
#define EAS_ELEMENT_TITLE                     "Title"
#define EAS_ELEMENT_SUFFIX                    "Suffix"

#define EAS_ELEMENT_HOMECITY                  "HomeCity"
#define EAS_ELEMENT_HOMECOUNTRY               "HomeCountry"
#define EAS_ELEMENT_HOMEPOSTALCODE            "HomePostalCode"
#define EAS_ELEMENT_HOMESTATE                 "HomeState"
#define EAS_ELEMENT_HOMESTREET                "HomeStreet"

#define EAS_ELEMENT_BUSINESSCITY              "BusinessCity"
#define EAS_ELEMENT_BUSINESSCOUNTRY           "BusinessCountry"
#define EAS_ELEMENT_BUSINESSPOSTALCODE        "BusinessPostalCode"
#define EAS_ELEMENT_BUSINESSSTATE             "BusinessState"
#define EAS_ELEMENT_BUSINESSSTREET            "BusinessStreet"

#define EAS_ELEMENT_OTHERCITY                 "OtherCity"
#define EAS_ELEMENT_OTHERCOUNTRY              "OtherCountry"
#define EAS_ELEMENT_OTHERPOSTALCODE           "OtherPostalCode"
#define EAS_ELEMENT_OTHERSTATE                "OtherState"
#define EAS_ELEMENT_OTHERSTREET               "OtherStreet"

#define EAS_ELEMENT_BUSINESSPHONENUMBER       "BusinessPhoneNumber"
#define EAS_ELEMENT_BUSINESS2PHONENUMBER      "Business2PhoneNumber"
#define EAS_ELEMENT_BUSINESSFAXNUMBER         "BusinessFaxNumber"
#define EAS_ELEMENT_HOMEPHONENUMBER           "HomePhoneNumber"
#define EAS_ELEMENT_HOME2PHONENUMBER          "Home2PhoneNumber"
#define EAS_ELEMENT_HOMEFAXNUMBER             "HomeFaxNumber"
#define EAS_ELEMENT_MOBILEPHONENUMBER         "MobilePhoneNumber"
#define EAS_ELEMENT_CARPHONENUMBER            "CarPhoneNumber"
#define EAS_ELEMENT_RADIOPHONENUMBER          "RadioPhoneNumber"

#define EAS_ELEMENT_EMAIL1ADDRESS             "Email1Address"
#define EAS_ELEMENT_EMAIL2ADDRESS             "Email2Address"
#define EAS_ELEMENT_EMAIL3ADDRESS             "Email3Address"

#define EAS_ELEMENT_BIRTHDAY                  "Birthday"
#define EAS_ELEMENT_URL                       "WebPage"
#define EAS_ELEMENT_NICKNAME                  "Alias"
#define EAS_ELEMENT_ORG                       "CompanyName"
#define EAS_ELEMENT_PAGER                     "PagerNumber"
#define EAS_ELEMENT_ROLE                      "JobTitle"
#define EAS_ELEMENT_PHOTO                     "Picture" 	/* VCard name: "PHOTO" */
#define EAS_ELEMENT_NOTE                      "Body"


//@
//TODO:
/* ActiveSync protocol message definition for contact */
/* targetNamespace for Contacts (from AS contacts schema) */
#define EAS_NAMESPACE_CONTACTS					"contacts:"
 
#define EAS_ELEMENT_ANNIVERSARY					"Anniversary"
#define EAS_ELEMENT_ASSISTANTNAME				"AssistantName"
#define EAS_ELEMENT_ASSISTANTPHONENUMBER		"AssistantPhoneNumber"
#define EAS_ELEMENT_DEPARTMENT					"Department"
#define EAS_ELEMENT_WEBPAGE						"WebPage"
#define EAS_ELEMENT_FILEAS						"FileAs"
#define EAS_ELEMENT_WEIGHTEDRANK				"WeightedRank"

#define EAS_ELEMENT_COMPANYNAME					"CompanyName"

#define EAS_ELEMENT_SPOUSE						"Spouse"
#define EAS_ELEMENT_JOBTITLE					"JobTitle"

#define EAS_ELEMENT_YOMIFIRSTNAME				"YomiFirstName"
#define EAS_ELEMENT_YOMILASTNAME				"YomiLastName"
#define EAS_ELEMENT_YOMICOMPANYNAME				"YomiCompanyName"

#define EAS_ELEMENT_OFFICELOCATION				"OfficeLocation"
#define EAS_ELEMENT_PICTURE						"Picture"
#define EAS_ELEMENT_CATEGORIES					"Categories"
#define EAS_ELEMENT_CATEGORY					"Category"
#define EAS_ELEMENT_CHILDREN					"Children"
#define EAS_ELEMENT_CHILD						"Child"

//#define EAS_ELEMENT_BODY						"Body"
/* targetNamespace for Contacts2 (from AS contacts schema) */
#define EAS_NAMESPACE_CONTACTS2					"contacts2:"
#define EAS_ELEMENT_CONTACTS2_CUSTOMERID		"CustomerId"
#define EAS_ELEMENT_CONTACTS2_GOVERNMENTID		"GovernmentId"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS			"IMAddress"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS2		"IMAddress2"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS3		"IMAddress3"
#define EAS_ELEMENT_CONTACTS2_MANAGERNAME		"ManagerName"
#define EAS_ELEMENT_CONTACTS2_COMPANYMAINPHONE	"CompanyMainPhone"
#define EAS_ELEMENT_CONTACTS2_ACCOUNTNAME		"AccountName"
#define EAS_ELEMENT_CONTACTS2_NICKNAME			"NickName"
#define EAS_ELEMENT_CONTACTS2_MMS				"MMS"


/* vCard attribute names and types*/
/* This list is an addition to the one defined in e-vcard.h */
#define VC_ATT_NAME_BEGIN	"BEGIN"



static void add_attr_value(EVCardAttribute *attr,xmlNodePtr node,const gchar *sought)
{
	xmlNodePtr n = node;
	gchar *value = NULL; 

	// find sought value 
	while (n) 
	{
		if (!xmlStrcmp((const xmlChar*)(n->name), (const xmlChar *)sought))
		{
			value = (gchar *)xmlNodeGetContent(n);
			break;
		}
		n = n->next;
	}
	// drop out if not found
	if(!value)
		return;
	// add sought value
	e_vcard_attribute_add_value(attr, value);
}

static void add_name_attr_values(EVCardAttribute *attr,xmlNodePtr node)
{
	add_attr_value(attr, node, EAS_ELEMENT_FIRSTNAME);
	add_attr_value(attr, node, EAS_ELEMENT_MIDDLENAME);
	add_attr_value(attr, node, EAS_ELEMENT_LASTNAME);
	add_attr_value(attr, node, EAS_ELEMENT_TITLE);
	add_attr_value(attr, node, EAS_ELEMENT_SUFFIX);
}

static void add_home_address_attr_values(EVCardAttribute *attr,xmlNodePtr node)
{
	add_attr_value(attr, node, EAS_ELEMENT_HOMECITY);
	add_attr_value(attr, node, EAS_ELEMENT_HOMECOUNTRY);
	add_attr_value(attr, node, EAS_ELEMENT_HOMEPOSTALCODE);
	add_attr_value(attr, node, EAS_ELEMENT_HOMESTATE);
	add_attr_value(attr, node, EAS_ELEMENT_HOMESTREET);
}

static void add_business_address_attr_values(EVCardAttribute *attr,xmlNodePtr node)
{
	add_attr_value(attr, node, EAS_ELEMENT_BUSINESSCITY);
	add_attr_value(attr, node, EAS_ELEMENT_BUSINESSCOUNTRY);
	add_attr_value(attr, node, EAS_ELEMENT_BUSINESSPOSTALCODE);
	add_attr_value(attr, node, EAS_ELEMENT_BUSINESSSTATE);
	add_attr_value(attr, node, EAS_ELEMENT_BUSINESSSTREET);
}

static void add_other_address_attr_values(EVCardAttribute *attr,xmlNodePtr node)
{
	add_attr_value(attr, node, EAS_ELEMENT_OTHERCITY);
	add_attr_value(attr, node, EAS_ELEMENT_OTHERCOUNTRY);
	add_attr_value(attr, node, EAS_ELEMENT_OTHERPOSTALCODE);
	add_attr_value(attr, node, EAS_ELEMENT_OTHERSTATE);
	add_attr_value(attr, node, EAS_ELEMENT_OTHERSTREET);
}
	
gchar* eas_con_info_translator_parse_response(xmlNodePtr node, 
                                              gchar* server_id)
{
	// Variable for the return value
	gchar* result = NULL;
	EVCard *vcard;
    EasItemInfo* conInfo = NULL;

    g_debug("eas_con_info_translator_parse_response ++");

	if (node && (node->type == XML_ELEMENT_NODE) && (!g_strcmp0((char*)(node->name), EAS_ELEMENT_APPLICATIONDATA)))
	{
		xmlNodePtr n = node;

		gboolean nameElements = FALSE;
		gboolean homeAddrElements = FALSE;
		gboolean workAddrElements = FALSE;
		gboolean otherAddrElements = FALSE;

		// vCard object
		vcard = e_vcard_new();

		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);
				EVCardAttributeParam *param = e_vcard_attribute_param_new("TYPE");
				EVCardAttributeParam *param2 = e_vcard_attribute_param_new("TYPE");
				
				//
				// Name elements
				//				
				if (((g_strcmp0(name, EAS_ELEMENT_FIRSTNAME) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_MIDDLENAME) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_LASTNAME) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_TITLE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_SUFFIX) == 0))
				    && (nameElements == FALSE))
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_N);
					add_name_attr_values(attr, node->children);
					e_vcard_add_attribute(vcard, attr);
					nameElements = TRUE;
				}

				//
				// Home Address elements
				//
				else if (((g_strcmp0(name, EAS_ELEMENT_HOMECITY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_HOMECOUNTRY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_HOMEPOSTALCODE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_HOMESTATE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_HOMESTREET) == 0))
				    && (homeAddrElements == FALSE))
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ADR);
					add_home_address_attr_values(attr, node->children);
					e_vcard_add_attribute(vcard, attr);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
					homeAddrElements = TRUE;
				}

				//
				// Business Address elements
				//
				else if (((g_strcmp0(name, EAS_ELEMENT_BUSINESSCITY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_BUSINESSCOUNTRY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_BUSINESSPOSTALCODE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_BUSINESSSTATE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_BUSINESSSTREET) == 0))
				    && (workAddrElements == FALSE))
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ADR);
					add_business_address_attr_values(attr, node->children);
					e_vcard_add_attribute(vcard, attr);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
					workAddrElements = TRUE;
				}

				//
				// Other Address elements
				//
				else if (((g_strcmp0(name, EAS_ELEMENT_OTHERCITY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_OTHERCOUNTRY) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_OTHERPOSTALCODE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_OTHERSTATE) == 0) ||
				    (g_strcmp0(name, EAS_ELEMENT_OTHERSTREET) == 0))
				    && (otherAddrElements == FALSE))
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ADR);
					add_other_address_attr_values(attr, node->children);
					e_vcard_add_attribute(vcard, attr);
					e_vcard_attribute_add_param_with_value(attr, param, "OTHER");
					otherAddrElements = TRUE;
				}
				
				//
				// Contact number elements
				//
				else if (g_strcmp0(name, EAS_ELEMENT_BUSINESSPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESSPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_BUSINESS2PHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESS2PHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_BUSINESSFAXNUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESSFAXNUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
					e_vcard_attribute_add_param_with_value(attr, param2, "FAX");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_HOMEPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOMEPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_HOME2PHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOME2PHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_HOMEFAXNUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOMEFAXNUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
					e_vcard_attribute_add_param_with_value(attr, param2, "FAX");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_MOBILEPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_MOBILEPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CELL");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_CARPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_CARPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CAR");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_RADIOPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_RADIOPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CELL");
				}
				else if (g_strcmp0(name, EAS_ELEMENT_PAGER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_PAGER);
					e_vcard_attribute_add_param_with_value(attr, param, "PAGER");
				}

				//
				// Email
				//
				
				else if (g_strcmp0(name, EAS_ELEMENT_EMAIL1ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL1ADDRESS);
				}
				else if (g_strcmp0(name, EAS_ELEMENT_EMAIL2ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL2ADDRESS);
				}
				else if (g_strcmp0(name, EAS_ELEMENT_EMAIL3ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL3ADDRESS);
				}

				//
				// Birthday
				//
				else if (g_strcmp0(name, EAS_ELEMENT_BIRTHDAY) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_BDAY);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BIRTHDAY);
				}

				//
				// URL
				//
				else if (g_strcmp0(name, EAS_ELEMENT_URL) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_URL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_URL);
				}

				//
				// Nickname
				//
				else if (g_strcmp0(name, EAS_ELEMENT_NICKNAME) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_NICKNAME);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_NICKNAME);
				}

				//
				// Org
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ORG) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ORG);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_ORG);
				}

				//
				// Role
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ROLE) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ROLE);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_ROLE);
				}

				//
				// Photo
				//
				else if (g_strcmp0(name, EAS_ELEMENT_PHOTO) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_PHOTO);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_PHOTO);
				}

				//
				// Note
				//
				else if (g_strcmp0(name, EAS_ELEMENT_NOTE) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_NOTE);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_NOTE);
				}
			}			
		}		
	}
	conInfo = eas_item_info_new();
	conInfo->server_id = (gchar*)server_id;
	conInfo->data = g_strdup(e_vcard_to_string(vcard, EVC_FORMAT_VCARD_30));

	if (!eas_item_info_serialise(conInfo, &result))
	{
		// TODO: log error
		result = NULL;
	}
	// Free the EasItemInfo GObject
	g_object_unref(conInfo);
	g_debug("eas_con_info_translator_parse_response --");
	return result;	
}


 /* ------------------------------------------------------ */
 /* Functionality to deal with Contacts request translator */
 /* -------------------------------------------------------*/
static const char *
property_get_nth_value(EVCardAttributeParam *param, int nth)
{
	const char *ret = NULL;
	GList *values = e_vcard_attribute_param_get_values(param);
	if (!values)
		return NULL;
	ret = g_list_nth_data(values, nth);
	//g_list_free(values);
	return ret;
}

static const char *
attribute_get_nth_value(EVCardAttribute *attr, int nth)
{
	GList *values = NULL;
	GString *retstr = NULL;
	
	values = e_vcard_attribute_get_values_decoded(attr);
	if (!values)
		return NULL;
	retstr = (GString *)g_list_nth_data(values, nth);
	if (!retstr)
		return NULL;

	if (!g_utf8_validate(retstr->str, -1, NULL)) {
		values = e_vcard_attribute_get_values(attr);
		if (!values)
			return NULL;
		return g_list_nth_data(values, nth);
	}

	return retstr->str;
}

typedef enum _EncodingType {
	EAS_UNKNOWN,
	EAS_BASE64,
	EAS_8BIT
} EncodingType;

/* Address type as defined by ActiveSync */
typedef enum _EasAddType {
	EAS_ADD_UNKNOWN,
	EAS_ADD_WORK,
	EAS_ADD_HOME,
	EAS_ADD_OTHER
} EasAddType;


static void
set_xml_element(xmlNodePtr appData, const xmlChar* name, const xmlChar* value, EncodingType encodingType)
{
	if (encodingType == EAS_BASE64)
	{
		gchar* base64value = NULL;

		//g_debug("base64encoder: size = %d", strlen(value));

		base64value = g_base64_encode((const guchar *)(&value), sizeof(value));
		xmlNewTextChild(appData, NULL, (const xmlChar*) name, (const xmlChar*)base64value);

		//g_debug ("data ecoded length  =--->:[%d]",  strlen (base64value));
        //g_debug ("data encoded   =--->:[%s]",   base64value);

		g_free(base64value);
	}
	else
	{
		xmlNewTextChild(appData, NULL, name, value);
	}
}

static EncodingType
get_encoding_type(EVCardAttribute *attr)
{
	GList *params = NULL;
	GList *p = NULL;
	EncodingType encoding;
	encoding  = EAS_8BIT;

	params = e_vcard_attribute_get_params(attr);

	for (p = params; p; p = p->next)
	{
		EVCardAttributeParam *param = p->data;
		if (!strcmp("ENCODING", e_vcard_attribute_param_get_name(param)))
		{
			if (!g_ascii_strcasecmp(property_get_nth_value(param, 0), "b"))
				encoding = EAS_BASE64;
		}
	}
	return encoding;
}

/* Check if an contact field allready set in the applicationdata xml children*/
static gboolean
is_element_set(xmlNodePtr appData, const gchar* name)
{
	xmlNodePtr node = NULL;
	node = appData;

	for(node = appData->children; node ; node = node->next)
		if(!strcmp((char*) node->name, name))
			return TRUE;

	return FALSE;
}

static void 
set_xml_address(xmlNodePtr appData, EVCardAttribute *attr, EasAddType easAddType, EncodingType encoding)
{

/*
	ActiveSync protocol limits the addresses to 3 only:
	 -one for home,
	 -one for business,
	 -one for other.

	vcard defines the following values in sequence: 
	post office box;
	extended address;
	street address;
	locality (e.g., city);
	region (e.g., state or province);
	postal code;
	country name.
*/
	
	if (easAddType == EAS_ADD_WORK)
	{
		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "PostalBox",
        (const xmlChar*)attribute_get_nth_value(attr, 0), encoding); */
		g_warning("TODO:AS Does not support PostalBox");

		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "ExtendedAddress",
        (const xmlChar*)attribute_get_nth_value(attr, 1), encoding); */
		g_warning("TODO:AS Does not support ExtendedAddress");

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESSSTREET,
        (const xmlChar*)attribute_get_nth_value(attr, 2), encoding);

		set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_BUSINESSCITY,
        (const xmlChar*)attribute_get_nth_value(attr, 3), encoding);

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESSSTATE,
        (const xmlChar*)attribute_get_nth_value(attr, 4), encoding);

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESSPOSTALCODE,
        (const xmlChar*)attribute_get_nth_value(attr, 5), encoding);

		set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_BUSINESSCOUNTRY,
        (const xmlChar*)attribute_get_nth_value(attr, 6), encoding);
	}
	else if (easAddType == EAS_ADD_HOME)
	{
		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "PostalBox",
        (const xmlChar*)attribute_get_nth_value(attr, 0), encoding); */
		g_warning("TODO:AS Does not support PostalBox");

		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "ExtendedAddress",
        (const xmlChar*)attribute_get_nth_value(attr, 1), encoding); */
			g_warning("TODO:AS Does not support ExtendedAddress");

		set_xml_element(appData, (const xmlChar*)EAS_ELEMENT_HOMESTREET,
        (const xmlChar*)attribute_get_nth_value(attr, 2), encoding);

		set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_HOMECITY,
        (const xmlChar*)attribute_get_nth_value(attr, 3), encoding);
	
		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOMESTATE,
        (const xmlChar*)attribute_get_nth_value(attr, 4), encoding);
	
		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOMEPOSTALCODE,
        (const xmlChar*)attribute_get_nth_value(attr, 5), encoding);
	
		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOMECOUNTRY,
        (const xmlChar*)attribute_get_nth_value(attr, 6), encoding);
	}
	else
	{
		/* deal with possible other vCard type of addresses:
		 "dom", "intl", "postal", "parcel", "pref" / iana-type / x-name */

		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "PostalBox",
        (const xmlChar*)attribute_get_nth_value(attr, 0), encoding); */
		g_warning("TODO:AS Does not support PostalBox");

		/* set_xml_element(appData, (const xmlChar*) EAS_NAMESPACE_CONTACTS "ExtendedAddress",
        (const xmlChar*)attribute_get_nth_value(attr, 1), encoding); */
		g_warning("TODO:AS Does not support ExtendedAddress");

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_OTHERSTREET,
        (const xmlChar*)attribute_get_nth_value(attr, 2), encoding);
		
		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_OTHERCITY,
        (const xmlChar*)attribute_get_nth_value(attr, 3), encoding);

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_OTHERSTATE,
        (const xmlChar*)attribute_get_nth_value(attr, 4), encoding);

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_OTHERPOSTALCODE,
        (const xmlChar*)attribute_get_nth_value(attr, 5), encoding);

		set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_OTHERCOUNTRY,
        (const xmlChar*)attribute_get_nth_value(attr, 6), encoding);
	}

}


static void
set_xml_tel(xmlNodePtr appData, EVCardAttribute *attr, EVCardAttributeParam *param, EncodingType encoding)
{
	const char *propname0 = NULL;
	const char *propname1 = NULL;
	
	propname0 = property_get_nth_value(param, 0);
	propname1 = property_get_nth_value(param, 1);

	if(strcmp(propname0, "WORK") == 0 && strcmp(propname1, "VOICE") == 0)
	{	
		if(!is_element_set(appData, EAS_ELEMENT_BUSINESSPHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESSPHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
		else if(!is_element_set(appData, EAS_ELEMENT_BUSINESS2PHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESS2PHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if(strcmp(propname0, "WORK") == 0 && strcmp(propname1, "FAX") == 0)
	{	
		if(!is_element_set(appData, EAS_ELEMENT_BUSINESSFAXNUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BUSINESSFAXNUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if(strcmp(propname0, "HOME") == 0 && strcmp(propname1, "VOICE") == 0)
	{
		if(!is_element_set(appData, EAS_ELEMENT_HOMEPHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOMEPHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
		else if(!is_element_set(appData, EAS_ELEMENT_HOME2PHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOME2PHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if(strcmp(propname0, "HOME") == 0 && strcmp(propname1, "FAX") == 0)
	{
		if(!is_element_set(appData, EAS_ELEMENT_HOMEFAXNUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_HOMEFAXNUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if (!strcmp(propname0, "CELL"))
	{
		if(!is_element_set(appData, EAS_ELEMENT_MOBILEPHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_MOBILEPHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if (!strcmp(propname0, "CAR"))
	{
		if(!is_element_set(appData, EAS_ELEMENT_CARPHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_CARPHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if (!strcmp(propname0, "RADIO"))
	{
		if(!is_element_set(appData, EAS_ELEMENT_RADIOPHONENUMBER))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_RADIOPHONENUMBER,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else
		g_debug("Tel Type not supported by ActiveSync: %s", propname0);

}

static void
set_xml_email(xmlNodePtr appData, EVCardAttribute *attr, EVCardAttributeParam *param, EncodingType encoding)
{
	const char *propname0 = NULL;
	propname0 = property_get_nth_value(param, 0);

	 if (!strcmp(propname0, "WORK"))
	{
		if(!is_element_set(appData, EAS_ELEMENT_EMAIL1ADDRESS))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_EMAIL1ADDRESS,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if (!strcmp(propname0, "HOME"))
	{
		if(!is_element_set(appData, EAS_ELEMENT_EMAIL2ADDRESS))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_EMAIL2ADDRESS,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else if (!strcmp(propname0, "OTHER") || !strcmp(propname0, "INTERNET"))
	{ 
		if(!is_element_set(appData, EAS_ELEMENT_EMAIL3ADDRESS))
		{
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_EMAIL3ADDRESS,
				(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
		}
	}
	else
		g_debug("Email type not Supported by ActiveSync: %s", propname0);
	
}

gboolean 
eas_con_info_translator_parse_request(	xmlDocPtr doc, 
										xmlNodePtr appData,
										EasItemInfo* contactInfo)
{
	gboolean success = FALSE;
	EVCard* vcard = NULL;
	GList *p = NULL;
	GList *a = NULL;
	GList *attributes = NULL;
	EncodingType encoding;

	g_return_val_if_fail (doc != NULL && appData != NULL && contactInfo != NULL, FALSE);
	g_return_val_if_fail (appData->type == XML_ELEMENT_NODE, FALSE);
	g_return_val_if_fail (appData->name != NULL, FALSE);
	g_return_val_if_fail (g_strcmp0((char*)(appData->name), EAS_ELEMENT_APPLICATIONDATA) == 0, FALSE);

	g_debug("eas_con_info_translator_parse_request ++");

	g_type_init();

	vcard = e_vcard_new_from_string (contactInfo->data);
	g_return_val_if_fail (vcard != NULL, FALSE);

	e_vcard_dump_structure (vcard); /* TODO: Debug only*/
	
	attributes = e_vcard_get_attributes(vcard);

	for (a = attributes; a; a = a->next) {
		const char *name = NULL;
		GList *params= NULL; 
		EVCardAttribute *attr = a->data;
		encoding = get_encoding_type(attr);
		name = e_vcard_attribute_get_name(attr);

		g_debug("e_vcard_attribute_get_name=%s", name);

#if 0
		if (!strcmp(name, "BEGIN"))
			continue;
#endif

		if (!strcmp(name, EVC_VERSION))
			continue;

		params = e_vcard_attribute_get_params(attr);

		/* process attributes that have no param */
		/* FullName */
		if (!strcmp(name, EVC_FN)) {
			//set_xml_element(appData, (const xmlChar*) "FullName", (const xmlChar*)attribute_get_nth_value(attr, 0));
			g_warning("TODO:AS Does not support FullName");
			continue;
		}

		/* Name */
		if (!strcmp(name, EVC_N)) {
			set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_LASTNAME,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_FIRSTNAME,
			                (const xmlChar*)attribute_get_nth_value(attr, 1), encoding);
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_MIDDLENAME,
							(const xmlChar*)attribute_get_nth_value(attr, 2), encoding);
			/*set_as_xml_element(appData, (const xmlChar*) "Prefix",
							(const xmlChar*)attribute_get_nth_value(attr, 3));*/
			g_warning("TODO: AS Does not support Prefix");
			set_xml_element(appData, (const xmlChar*)  EAS_ELEMENT_SUFFIX,
			                (const xmlChar*)attribute_get_nth_value(attr, 4), encoding);
			continue;
		}

		/* NICKNAME */
		/* TODO: -NickName (Contacts2)-> does not show in Exchange Outlook
		 		 -Alias -> causes AS malformed item error:
		  				MS-ASWBXML and wbxml (in wbxml_tables.c) indicate that this
						itme is not supported when the MS-ASProtocolVersion header is set to 12.1 */
		if (!strcmp(name, EVC_NICKNAME)) {
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_CONTACTS2_NICKNAME,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}
		
		/* Company */
		if (!strcmp(name, EVC_ORG)) {
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_COMPANYNAME,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			/*set_xml_element(appData, (const xmlChar*)EAS_NAMESPACE_CONTACTS "Unit",
							(const xmlChar*)attribute_get_nth_value(attr, 1), encoding);*/
			g_warning("TODO:AS Does not support ORG->Unit");
			continue;
		}

		
		/* Url */
		if (!strcmp(name, EVC_URL)) {
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_WEBPAGE,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}

		/* Birthday */
		if (!strcmp(name, EVC_BDAY)) {
			//set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_BIRTHDAY,
			//                (const xmlChar*)/*attribute_get_nth_value(attr, 0)*/"20110628T000000", encoding);
			continue;
		}
		
		/* Title */
		if (!strcmp(name, EVC_TITLE)) {
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_TITLE,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}

		/* Role */
		if (!strcmp(name, EVC_ROLE)) {
			set_xml_element(appData, (const xmlChar*) EAS_ELEMENT_JOBTITLE,
			                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}

		/* Uid */
		if (!strcmp(name, EVC_UID)) {
			/*set_xml_element(appData, (const xmlChar*) "Uid",
						(const xmlChar*)attribute_get_nth_value(attr, 0), encoding);*/
			g_warning("TODO:AS Does not support Uid");
			continue;
		}

		/* Note */
		if (!strcmp(name, EVC_NOTE)) {
			//set_xml_element(appData, (const xmlChar*)EAS_ELEMENT_NOTE,
			//                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}

		/* Photo (AS: name is "Picture") */
		if (!strcmp(name, EVC_PHOTO)) {
			set_xml_element(appData, (const xmlChar*)EAS_ELEMENT_PICTURE,
		                (const xmlChar*)attribute_get_nth_value(attr, 0), encoding);
			continue;
		}

		/* process attributes that have param */
		for (p = params; p; p = p->next) {
			EVCardAttributeParam *param = p->data;
			/* Address */
			if (!strcmp(name, EVC_ADR)) {
				const char *propname = property_get_nth_value(param, 0);
				if (!strcmp(propname, "WORK"))
					set_xml_address(appData, attr, EAS_ADD_WORK, encoding);
				else if (!strcmp(propname, "HOME"))
					set_xml_address(appData, attr, EAS_ADD_HOME, encoding);
				else
					set_xml_address(appData, attr, EAS_ADD_OTHER, encoding);

				continue;
			}

			/* Address Labeling */
			if (!strcmp(name, EVC_LABEL)) {
				g_warning("TODO: LABEL  not implemented yet");
				continue;
			}

			/* Telephone */
			if (!strcmp(name, EVC_TEL)) {
				set_xml_tel(appData, attr, param, encoding);
				continue;
			}

			/* EMail */
			if (!strcmp(name, EVC_EMAIL )) {
				set_xml_email(appData, attr, param, encoding);
				continue;
			}

			/* Mailer */
			if (!strcmp(name, EVC_MAILER)) {
				g_warning("TODO: MAILER not implemented yet");
				continue;
			}

			/* Timezone */
			if (!strcmp(name, "TZ")) {
				g_warning("TODO:AS Does not support TZ");
				continue;
			}

			/* Location */
			if (!strcmp(name, EVC_GEO)) {
				g_warning("TODO:AS Does not support GEO");
				continue;
			}

			/* Logo */
			if (!strcmp(name, EVC_LOGO)) {
				g_warning("TODO:AS Does not support LOGO");
				continue;
			}

			/* Revision */
			if (!strcmp(name, EVC_REV)) {
				g_warning("AS Does not support %s", name);
				//set_xml_element(appData, (const xmlChar*)EAS_NAMESPACE_CONTACTS "Revision", (const xmlChar*)attribute_get_nth_value(attr, 0));
				continue;
			}

			/* Sound */
			if (!strcmp(name, "SOUND")) {
				g_warning("TODO:AS Does not support SOUND");
				continue;
			}

			/* Public Key */
			if (!strcmp(name, EVC_KEY)) {
				g_warning("TODO:AS Does not support KEY");
				//set_xml_element(appData, (const xmlChar*)EAS_NAMESPACE_CONTACTS "Key", (const xmlChar*)attribute_get_nth_value(attr, 0));
				continue;
			}
		}
	}


	if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 4))
	{
		xmlChar* dump_buffer = NULL;
		int dump_buffer_size = 0;
		xmlIndentTreeOutput = 1;
		xmlDocDumpFormatMemory (doc, &dump_buffer, &dump_buffer_size, 1);
		g_debug ("XML DOCUMENT DUMPED:\n%s", dump_buffer);
		xmlFree (dump_buffer);
	}

	g_debug("eas_con_info_translator_parse_request --");
	return success;
}
