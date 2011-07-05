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
#define EAS_ELEMENT_HOMESTATE                 "HomeRegion"
#define EAS_ELEMENT_HOMESTREET                "HomeStreet"

#define EAS_ELEMENT_BUSINESSCITY              "BusinessCity"
#define EAS_ELEMENT_BUSINESSCOUNTRY           "BusinessCountry"
#define EAS_ELEMENT_BUSINESSPOSTALCODE        "BusinessPostalCode"
#define EAS_ELEMENT_BUSINESSSTATE             "BusinessRegion"
#define EAS_ELEMENT_BUSINESSSTREET            "BusinessStreet"

#define EAS_ELEMENT_OTHERCITY                 "OtherCity"
#define EAS_ELEMENT_OTHERCOUNTRY              "OtherCountry"
#define EAS_ELEMENT_OTHERPOSTALCODE           "OtherPostalCode"
#define EAS_ELEMENT_OTHERSTATE                "OtherRegion"
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
#define EAS_ELEMENT_PHOTO                     "Picture"
#define EAS_ELEMENT_NOTE                      "airsyncbase:Body"

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

		gboolean nameElements = FALSE;
		gboolean homeAddrElements = FALSE;
		gboolean workAddrElements = FALSE;
		gboolean otherAddrElements = FALSE;

		// vCard object
		vcard = e_vcard_new();
//		nameProp = addProp(vcard,VCNameProp);

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
				if (((g_strcmp0(name, EAS_ELEMENT_HOMECITY) == 0) ||
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
				if (((g_strcmp0(name, EAS_ELEMENT_BUSINESSCITY) == 0) ||
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
				if (((g_strcmp0(name, EAS_ELEMENT_OTHERCITY) == 0) ||
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
				if (g_strcmp0(name, EAS_ELEMENT_BUSINESSPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESSPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
				}
				if (g_strcmp0(name, EAS_ELEMENT_BUSINESS2PHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESS2PHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
				}
				if (g_strcmp0(name, EAS_ELEMENT_BUSINESSFAXNUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BUSINESSFAXNUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "WORK");
					e_vcard_attribute_add_param_with_value(attr, param2, "FAX");
				}
				if (g_strcmp0(name, EAS_ELEMENT_HOMEPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOMEPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
				}
				if (g_strcmp0(name, EAS_ELEMENT_HOME2PHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOME2PHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
				}
				if (g_strcmp0(name, EAS_ELEMENT_HOMEFAXNUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_HOMEFAXNUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "HOME");
					e_vcard_attribute_add_param_with_value(attr, param2, "FAX");
				}
				if (g_strcmp0(name, EAS_ELEMENT_MOBILEPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_MOBILEPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CELL");
				}
				if (g_strcmp0(name, EAS_ELEMENT_CARPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_CARPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CAR");
				}
				if (g_strcmp0(name, EAS_ELEMENT_RADIOPHONENUMBER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_RADIOPHONENUMBER);
					e_vcard_attribute_add_param_with_value(attr, param, "CELL");
				}
				if (g_strcmp0(name, EAS_ELEMENT_PAGER) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_TEL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_PAGER);
					e_vcard_attribute_add_param_with_value(attr, param, "PAGER");
				}

				//
				// Email
				//
				
				if (g_strcmp0(name, EAS_ELEMENT_EMAIL1ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL1ADDRESS);
				}
				if (g_strcmp0(name, EAS_ELEMENT_EMAIL2ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL2ADDRESS);
				}
				if (g_strcmp0(name, EAS_ELEMENT_EMAIL3ADDRESS) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_EMAIL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_EMAIL3ADDRESS);
				}

				//
				// Birthday
				//
				if (g_strcmp0(name, EAS_ELEMENT_BIRTHDAY) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_BDAY);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_BIRTHDAY);
				}

				//
				// URL
				//
				if (g_strcmp0(name, EAS_ELEMENT_URL) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_URL);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_URL);
				}

				//
				// Nickname
				//
				if (g_strcmp0(name, EAS_ELEMENT_NICKNAME) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_NICKNAME);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_NICKNAME);
				}

				//
				// Org
				//
				if (g_strcmp0(name, EAS_ELEMENT_ORG) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ORG);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_ORG);
				}

				//
				// Role
				//
				if (g_strcmp0(name, EAS_ELEMENT_ROLE) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_ROLE);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_ROLE);
				}

				//
				// Photo
				//
				if (g_strcmp0(name, EAS_ELEMENT_PHOTO) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_PHOTO);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_PHOTO);
				}

				//
				// Note
				//
				if (g_strcmp0(name, EAS_ELEMENT_NOTE) == 0)
				{
					EVCardAttribute *attr = e_vcard_attribute_new(NULL, EVC_NOTE);

					e_vcard_add_attribute(vcard, attr);
					add_attr_value(attr,node->children,EAS_ELEMENT_NOTE);
				}
			}			
		}		
	}

	result = e_vcard_to_string(vcard, EVC_FORMAT_VCARD_30);

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
