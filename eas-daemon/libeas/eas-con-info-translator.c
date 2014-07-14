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

#include <libedataserver/eds-version.h>
#if EDS_CHECK_VERSION(3,6,0)
# include <libebook/libebook.h>
#else
# include <libebook/e-vcard.h>
#endif
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
#define EAS_ELEMENT_PAGER                     "PagerNumber"
#define EAS_ELEMENT_ROLE                      "JobTitle"
#define EAS_ELEMENT_PHOTO                     "Picture" 	/* VCard name: "PHOTO" */

#define EAS_NAMESPACE_AIRSYNCBASE             			"airsyncbase:"
#define EAS_ELEMENT_BODY					"Body"
#define EAS_ELEMENT_BODY_TYPE					"Type"
#define EAS_ELEMENT_BODY_SIZE					"EstimatedDataSize"
#define EAS_ELEMENT_BODY_TRUNCATED				"Truncated"
#define EAS_ELEMENT_BODY_DATA					"Data"
#define EAS_ELEMENT_BODY_PREVIEW				"Preview"

#define EAS_ELEMENT_ANNIVERSARY					"Anniversary"
#define EAS_ELEMENT_ASSISTANTNAME				"AssistantName"
#define EAS_ELEMENT_ASSISTANTPHONENUMBER		"AssistantPhoneNumber"
#define EAS_ELEMENT_DEPARTMENT					"Department"
#define EAS_ELEMENT_FILEAS						"FileAs"
#define EAS_ELEMENT_WEIGHTEDRANK				"WeightedRank"

#define EAS_ELEMENT_COMPANYNAME					"CompanyName"

#define EAS_ELEMENT_SPOUSE						"Spouse"

#define EAS_ELEMENT_YOMIFIRSTNAME				"YomiFirstName"
#define EAS_ELEMENT_YOMILASTNAME				"YomiLastName"
#define EAS_ELEMENT_YOMICOMPANYNAME				"YomiCompanyName"

#define EAS_ELEMENT_OFFICELOCATION				"OfficeLocation"
#define EAS_ELEMENT_CATEGORIES					"Categories"
#define EAS_ELEMENT_CATEGORY					"Category"
#define EAS_ELEMENT_CHILDREN					"Children"
#define EAS_ELEMENT_CHILD						"Child"


/* targetNamespace for Contacts2 (from AS contacts schema) */
#define EAS_ELEMENT_CONTACTS2_CUSTOMERID		"CustomerId"
#define EAS_ELEMENT_CONTACTS2_GOVERNMENTID		"GovernmentId"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS			"IMAddress"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS2		"IMAddress2"
#define EAS_ELEMENT_CONTACTS2_IMADDRESS3		"IMAddress3"
#define EAS_ELEMENT_CONTACTS2_MANAGERNAME		"ManagerName"
#define EAS_ELEMENT_CONTACTS2_COMPANYMAINPHONE		"CompanyMainPhone"
#define EAS_ELEMENT_CONTACTS2_ACCOUNTNAME		"AccountName"
#define EAS_ELEMENT_CONTACTS2_NICKNAME			"NickName"
#define EAS_ELEMENT_CONTACTS2_MMS				"MMS"

#define EAS_EMAIL_START "<"
#define EAS_EMAIL_END ">"

// helper function to trim a buffer based on a start string and an end string
// eg. '"john.doe@work.com" <john.doe@work.com>'  trims to 'john.doe@work.com'
// where start string is "<" and end string is ">"
// returns a newly allocated string which must be freed
static gchar *extract_addrspec(const gchar* buf)
{
	gchar *start, *end;

	/* FIXME: This will behave incorrectly given an address like
	   "Foo<bar" <foo@bar.com>
	   ... but at least it no longer segfaults. */
	start = strchr(buf, '<');
	if (!start)
		return g_strdup(buf);

	start++;
	end = strchr(start, '>');
	if (!end)
		return g_strdup(buf);

	return g_strndup(start, end - start);
}

static void
add_email_attr_value (EVCardAttribute *attr, xmlNodePtr node, const gchar *sought)
{
	xmlNodePtr n = node;
	gchar *value = NULL;

	// find sought value
	while (n) {
		if (!xmlStrcmp ( (const xmlChar*) (n->name), (const xmlChar *) sought)) {
			value = (gchar *) xmlNodeGetContent (n);
			break;
		}
		n = n->next;
	}

	if(value!=NULL){
		gchar* trimmedAddress = extract_addrspec(value);
		e_vcard_attribute_add_value (attr, trimmedAddress); // e_vcard copies value
		g_free(trimmedAddress);
	}else{
		/* if sought not found (i.e. value = NULL) then e_vcard_attribute_add_value()
		inserts a semicolon otherwise it adds the value*/
		e_vcard_attribute_add_value (attr, value); // e_vcard copies value
	}

	xmlFree (value);
}

static void
add_attr_value (EVCardAttribute *attr, xmlNodePtr node, const gchar *sought)
{
	xmlNodePtr n = node;
	gchar *value = NULL;

	// find sought value
	while (n) {
		if (!xmlStrcmp ( (const xmlChar*) (n->name), (const xmlChar *) sought)) {
			value = (gchar *) xmlNodeGetContent (n);
			break;
		}
		n = n->next;
	}

	/* if sought not found (i.e. value = NULL) then e_vcard_attribute_add_value()
	inserts a semicolon otherwise it adds the value*/
	e_vcard_attribute_add_value (attr, value); // e_vcard copies value
	xmlFree (value);
}

/** strip time if zero, otherwise convert back to vCard 3.0 date-time */
static void add_bday_attr_value (EVCardAttribute *attr, xmlNodePtr node)
{
	gchar* value = (gchar *) xmlNodeGetContent (node);
	if (value) {
		/* inverse operation of set_xml_contact_date():
		   YYYY-MM-DDT00:00:00.000Z -> YYYY-MM-DD (strip empty time)
		   YYYY-MM-DDThh:mm:ss.mmmZ -> YYYY-MM-DDThh:mm:ssZ */
		gchar* tmp = strchr (value, 'T');
		if (tmp) {
			if (!strcmp (tmp, "T00:00:00.000Z"))
				*tmp = 0;
			else if ((tmp = strchr (value, '.')) != NULL) {
				*tmp = 'Z';
				tmp++;
				*tmp = 0;
			}
		}
		e_vcard_attribute_add_value (attr, value);
		xmlFree (value);
	}
}

static void add_name_attr_values (EVCardAttribute *attr, xmlNodePtr node)
{
	add_attr_value (attr, node, EAS_ELEMENT_LASTNAME);
	add_attr_value (attr, node, EAS_ELEMENT_FIRSTNAME);
	add_attr_value (attr, node, EAS_ELEMENT_MIDDLENAME);
	/* Prefix not supported add ";" instead */
	e_vcard_attribute_add_value (attr, NULL);
	add_attr_value (attr, node, EAS_ELEMENT_SUFFIX);
}

static void add_home_address_attr_values (EVCardAttribute *attr, xmlNodePtr node)
{
	add_attr_value (attr, node, "POBox");
	add_attr_value (attr, node, "extended");
	add_attr_value (attr, node, EAS_ELEMENT_HOMESTREET);
    add_attr_value (attr, node, EAS_ELEMENT_HOMECITY);
	add_attr_value (attr, node, EAS_ELEMENT_HOMESTATE);
	add_attr_value (attr, node, EAS_ELEMENT_HOMEPOSTALCODE);
	add_attr_value (attr, node, EAS_ELEMENT_HOMECOUNTRY);
}

static void add_business_address_attr_values (EVCardAttribute *attr, xmlNodePtr node)
{
	add_attr_value (attr, node, "POBox");
	add_attr_value (attr, node, "extended");
	add_attr_value (attr, node, EAS_ELEMENT_BUSINESSSTREET);
    add_attr_value (attr, node, EAS_ELEMENT_BUSINESSCITY);
	add_attr_value (attr, node, EAS_ELEMENT_BUSINESSSTATE);
	add_attr_value (attr, node, EAS_ELEMENT_BUSINESSPOSTALCODE);
	add_attr_value (attr, node, EAS_ELEMENT_BUSINESSCOUNTRY);
}

static void add_other_address_attr_values (EVCardAttribute *attr, xmlNodePtr node)
{
	add_attr_value (attr, node, "POBox");
	add_attr_value (attr, node, "extended");
	add_attr_value (attr, node, EAS_ELEMENT_OTHERSTREET);
    add_attr_value (attr, node, EAS_ELEMENT_OTHERCITY);
	add_attr_value (attr, node, EAS_ELEMENT_OTHERSTATE);
	add_attr_value (attr, node, EAS_ELEMENT_OTHERPOSTALCODE);
	add_attr_value (attr, node, EAS_ELEMENT_OTHERCOUNTRY);
}

gchar* eas_con_info_translator_parse_response (xmlNodePtr node,
					       gchar* server_id)
{
	// Variable for the return value
	gchar* result = NULL;
	EVCard *vcard = NULL;
	EasItemInfo* conInfo = NULL;

	

		xmlNodePtr n = node;

		gboolean nameElements = FALSE;
		gboolean homeAddrElements = FALSE;
		gboolean workAddrElements = FALSE;
		gboolean otherAddrElements = FALSE;
		gboolean titleElements = FALSE;
		gboolean orgElements = FALSE;

	g_debug ("eas_con_info_translator_parse_response ++");

		// vCard object
		vcard = e_vcard_new();

		for (n = n->children; n; n = n->next) {
			if (n->type == XML_ELEMENT_NODE) {
				const gchar* name = (const gchar*) (n->name);

				//
				// Name elements
				//
				if ( ( (g_strcmp0 (name, EAS_ELEMENT_FIRSTNAME) == 0) ||
				       (g_strcmp0 (name, EAS_ELEMENT_MIDDLENAME) == 0) ||
				       (g_strcmp0 (name, EAS_ELEMENT_LASTNAME) == 0) ||
				       (g_strcmp0 (name, EAS_ELEMENT_SUFFIX) == 0))
				     && (nameElements == FALSE)) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_N);
					add_name_attr_values (attr, node->children);
					e_vcard_add_attribute (vcard, attr);
					nameElements = TRUE;

				} else if (g_strcmp0 (name, EAS_ELEMENT_TITLE) == 0 &&
					   (titleElements == FALSE)) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TITLE);
					add_attr_value (attr, node->children, EAS_ELEMENT_TITLE);
					e_vcard_add_attribute (vcard, attr);
					titleElements = TRUE;
				}
				//
				// Home Address elements
				//
				else if ( ( (g_strcmp0 (name, EAS_ELEMENT_HOMECITY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_HOMECOUNTRY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_HOMEPOSTALCODE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_HOMESTATE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_HOMESTREET) == 0))
					  && (homeAddrElements == FALSE)) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_ADR);

					add_home_address_attr_values (attr, node->children);
					e_vcard_add_attribute (vcard, attr);
					e_vcard_attribute_add_param_with_value (attr, param, "HOME");
					homeAddrElements = TRUE;
				}

				//
				// Business Address elements
				//
				else if ( ( (g_strcmp0 (name, EAS_ELEMENT_BUSINESSCITY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_BUSINESSCOUNTRY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_BUSINESSPOSTALCODE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_BUSINESSSTATE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_BUSINESSSTREET) == 0))
					  && (workAddrElements == FALSE)) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_ADR);

					add_business_address_attr_values (attr, node->children);
					e_vcard_add_attribute (vcard, attr);
					e_vcard_attribute_add_param_with_value (attr, param, "WORK");
					workAddrElements = TRUE;
				}

				//
				// Other Address elements
				//
				else if ( ( (g_strcmp0 (name, EAS_ELEMENT_OTHERCITY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_OTHERCOUNTRY) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_OTHERPOSTALCODE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_OTHERSTATE) == 0) ||
					    (g_strcmp0 (name, EAS_ELEMENT_OTHERSTREET) == 0))
					  && (otherAddrElements == FALSE)) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_ADR);

					add_other_address_attr_values (attr, node->children);
					e_vcard_add_attribute (vcard, attr);
					e_vcard_attribute_add_param_with_value (attr, param, "OTHER");
					otherAddrElements = TRUE;
				}

				//
				// Contact number elements
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_BUSINESSPHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_BUSINESSPHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "WORK");
				} else if (g_strcmp0 (name, EAS_ELEMENT_BUSINESS2PHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_BUSINESS2PHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "WORK");
				} else if (g_strcmp0 (name, EAS_ELEMENT_BUSINESSFAXNUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttributeParam *param2 = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_BUSINESSFAXNUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "WORK");
					e_vcard_attribute_add_param_with_value (attr, param2, "FAX");
				} else if (g_strcmp0 (name, EAS_ELEMENT_HOMEPHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_HOMEPHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "HOME");
				} else if (g_strcmp0 (name, EAS_ELEMENT_HOME2PHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_HOME2PHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "HOME");
				} else if (g_strcmp0 (name, EAS_ELEMENT_HOMEFAXNUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttributeParam *param2 = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_HOMEFAXNUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "HOME");
					e_vcard_attribute_add_param_with_value (attr, param2, "FAX");
				} else if (g_strcmp0 (name, EAS_ELEMENT_MOBILEPHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_MOBILEPHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "CELL");
				} else if (g_strcmp0 (name, EAS_ELEMENT_CARPHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_CARPHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, "CAR");
				} else if (g_strcmp0 (name, EAS_ELEMENT_RADIOPHONENUMBER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_RADIOPHONENUMBER);
					e_vcard_attribute_add_param_with_value (attr, param, EVC_X_RADIO /* X-EVOLUTION-RADIO */);
				} else if (g_strcmp0 (name, EAS_ELEMENT_PAGER) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_TEL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_PAGER);
					e_vcard_attribute_add_param_with_value (attr, param, "PAGER");
				}

				//
				// Email
				//

				else if (g_strcmp0 (name, EAS_ELEMENT_EMAIL1ADDRESS) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_EMAIL);

					e_vcard_add_attribute (vcard, attr);
					add_email_attr_value (attr, node->children, EAS_ELEMENT_EMAIL1ADDRESS);
					e_vcard_attribute_add_param_with_value (attr, param, "internet");
				} else if (g_strcmp0 (name, EAS_ELEMENT_EMAIL2ADDRESS) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_EMAIL);

					e_vcard_add_attribute (vcard, attr);
					add_email_attr_value (attr, node->children, EAS_ELEMENT_EMAIL2ADDRESS);
					e_vcard_attribute_add_param_with_value (attr, param, "internet");
				} else if (g_strcmp0 (name, EAS_ELEMENT_EMAIL3ADDRESS) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("TYPE");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_EMAIL);

					e_vcard_add_attribute (vcard, attr);
					add_email_attr_value (attr, node->children, EAS_ELEMENT_EMAIL3ADDRESS);
					e_vcard_attribute_add_param_with_value (attr, param, "internet");
				}

				//
				// Birthday
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_BIRTHDAY) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_BDAY);

					e_vcard_add_attribute (vcard, attr);
					add_bday_attr_value (attr, n);
				}

				//
				// URL
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_URL) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_URL);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_URL);
				}

				//
				// Nickname
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_CONTACTS2_NICKNAME) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_NICKNAME);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_CONTACTS2_NICKNAME);
				}

				//
				// Org
				//
				else if ( ( g_strcmp0 (name, EAS_ELEMENT_COMPANYNAME) == 0 ||
					    g_strcmp0 (name, EAS_ELEMENT_DEPARTMENT) == 0 ||
					    g_strcmp0 (name, EAS_ELEMENT_OFFICELOCATION) == 0 )
					  && !orgElements) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_ORG);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_COMPANYNAME);
					add_attr_value (attr, node->children, EAS_ELEMENT_DEPARTMENT);
					add_attr_value (attr, node->children, EAS_ELEMENT_OFFICELOCATION);
					orgElements = TRUE;
				}

				//
				// Role
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_ROLE) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_ROLE);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_ROLE);
				}

				else if (g_strcmp0 (name, EAS_ELEMENT_SPOUSE) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_X_SPOUSE);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_SPOUSE);
				}

				//
				// Photo
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_PHOTO) == 0) {
					EVCardAttributeParam *param = e_vcard_attribute_param_new ("ENCODING");
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_PHOTO);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_PHOTO);
					e_vcard_attribute_add_param_with_value (attr, param, "b");
				}

				//
				// FileAs
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_FILEAS) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_X_FILE_AS);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_FILEAS);
				}

				//
				// MANAGER
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_CONTACTS2_MANAGERNAME) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_X_MANAGER);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_CONTACTS2_MANAGERNAME);
				}
				
				//
				// ASSISTANT
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_ASSISTANTNAME) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_X_ASSISTANT);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_ASSISTANTNAME);
				}

				else if (g_strcmp0 (name, EAS_ELEMENT_ANNIVERSARY) == 0) {
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_X_ANNIVERSARY);

					e_vcard_add_attribute (vcard, attr);
					add_attr_value (attr, node->children, EAS_ELEMENT_ANNIVERSARY);
				}

				//
				// Note
				//
				else if (g_strcmp0 (name, EAS_ELEMENT_BODY) == 0) {
					xmlNodePtr subNode = NULL;
					for (subNode = n->children; subNode; subNode = subNode->next) {
						if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_BODY_DATA)) {
							EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_NOTE);
							e_vcard_add_attribute (vcard, attr);
							add_attr_value (attr, subNode, EAS_ELEMENT_BODY_DATA);
							break;
						}
					}
				} else if (g_strcmp0 (name, EAS_ELEMENT_CATEGORIES) == 0) {
					xmlNodePtr childNode = NULL;
					EVCardAttribute *attr = e_vcard_attribute_new (NULL, EVC_CATEGORIES);
					e_vcard_add_attribute (vcard, attr);

					childNode = n;
					for (childNode = n->children; childNode ; childNode = childNode->next)
						if (!g_strcmp0 ( (const char *) childNode->name, (const char *) EAS_ELEMENT_CATEGORY)) {
							gchar * value = (gchar *) xmlNodeGetContent (childNode);
							e_vcard_attribute_add_value (attr, value);
							xmlFree (value);
						}
				}
			}
		}
	
	conInfo = eas_item_info_new();
	conInfo->server_id = (gchar*) server_id;
	conInfo->data = e_vcard_to_string (vcard, EVC_FORMAT_VCARD_30); // no need to duplicate, e_vcard allocates memory
	g_object_unref (G_OBJECT (vcard));
	if (!eas_item_info_serialise (conInfo, &result)) {
		// TODO: log error
		result = NULL;
	}
	// Free the EasItemInfo GObject
	g_object_unref (conInfo);
	g_debug ("eas_con_info_translator_parse_response --");
	return result;
}

/** known TYPE values */
enum Type {
	OTHER,
	HOME,
	WORK,
	CAR,
	RADIO,
	CELL,
	VOICE,
	FAX,
	PAGER,
};

static int
string_to_type (const gchar *type)
{
	static struct map {
		const gchar *name;
		enum Type type;
	} map[] = {
#define entry(_x) { #_x, _x }
		entry(OTHER),
		entry(HOME),
		entry(WORK),
		entry(CAR),
		{ EVC_X_RADIO, RADIO },
		entry(CELL),
		entry(VOICE),
		entry(FAX),
		entry(PAGER),
#undef entry
		{ NULL }
	},
		  * entry;

	for (entry = map; type && entry->name; entry++) {
		// match literally or with enclosing quotation marks;
		// libebook writes TYPE="X-EVOLUTION-RADIO"
		size_t len = strlen(type);
		if (!g_ascii_strcasecmp (type, entry->name) ||
		    (len == strlen(entry->name) + 2 &&
		     type[0] == '"' &&
		     type[len - 1] == '"' &&
		     !g_ascii_strncasecmp (type + 1, entry->name, strlen(entry->name))))
			return 1 << entry->type;
	}
	return 0;
}

enum TypeMask {
#define entry(_x) _x ## _MASK = (1 << _x)
	entry(OTHER),
	entry(HOME),
	entry(WORK),
	entry(CAR),
	entry(RADIO),
	entry(CELL),
	entry(VOICE),
	entry(FAX),
	entry(PAGER)
#undef entry
};

/** gather bit map of types, regardless whether short form (TEL;WORK;VOICE)
    or long form (TEL;TYPE=WORK,VOICE) is used */
static int
attribute_get_type (EVCardAttribute *attr)
{
	int ret = 0;
	GList *params = e_vcard_attribute_get_params (attr);
	GList *p;
	int match;

	for (p = params; p; p = p->next) {
		EVCardAttributeParam *param = p->data;
		const gchar *name = e_vcard_attribute_param_get_name (param);
		if (name && !g_ascii_strcasecmp (name, "TYPE")) {
			GList *values = e_vcard_attribute_param_get_values (param);
			GList *v;
			for (v = values; v; v = v->next) {
				const gchar *value = v->data;
				match = string_to_type (value);
				ret |= match;
			}
		} else {
			match = string_to_type (name);
			ret |= match;
		}
	}
	return ret;
}

static const char *
attribute_get_nth_value (EVCardAttribute *attr, int nth)
{
	GList *values = NULL;

	values = e_vcard_attribute_get_values (attr);
	if (!values)
		return NULL;

	return g_list_nth_data (values, nth);
}

/* Check if a contact field allready set in the applicationdata xml children*/
static gboolean
is_element_set (xmlNodePtr appData, const gchar* name)
{
	xmlNodePtr node = NULL;
	g_return_val_if_fail (appData != NULL && name != NULL, FALSE);
	node = appData;
	for (node = appData->children; node ; node = node->next)
		if (!strcmp ( (char*) node->name, name))
			return TRUE;

	return FALSE;
}

static void
set_xml_element (xmlNodePtr appData, const xmlChar* name, const xmlChar* content)
{
	g_return_if_fail (appData->type == XML_ELEMENT_NODE);
	g_return_if_fail (appData->name != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (content != NULL);

	xmlNewTextChild (appData, NULL, name, content);
}

static void
set_xml_name (xmlNodePtr appData, EVCardAttribute *attr)
{
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_LASTNAME,
			 (const xmlChar*) attribute_get_nth_value (attr, 0));
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_FIRSTNAME,
			 (const xmlChar*) attribute_get_nth_value (attr, 1));
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_MIDDLENAME,
			 (const xmlChar*) attribute_get_nth_value (attr, 2));
	/* ActiveSync does not support Prefix */
	/* set_xml_element(appData, (const xmlChar*) "Prefix",
	                (const xmlChar*)attribute_get_nth_value(attr, 3)); */
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_SUFFIX,
			 (const xmlChar*) attribute_get_nth_value (attr, 4));
}

static void
set_xml_org (xmlNodePtr appData, EVCardAttribute *attr)
{
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_COMPANYNAME,
			 (const xmlChar*) attribute_get_nth_value (attr, 0));
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_DEPARTMENT,
			 (const xmlChar*) attribute_get_nth_value (attr, 1));
	set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_OFFICELOCATION,
			 (const xmlChar*) attribute_get_nth_value (attr, 2));
}

static void
set_xml_address2 (xmlNodePtr appData, EVCardAttribute *attr, const xmlChar* add0,
		  const xmlChar* add1, const xmlChar* add2, const xmlChar* add3,
		  const xmlChar* add4, const xmlChar* add5, const xmlChar* add6)
{
	/* cannot add already existing type of address again */
	if (is_element_set (appData, (const gchar *)add2))
		return;

#if 0
	/ * AS does not support PostalBox * /
	set_xml_element (appData, add0, (const xmlChar*) attribute_get_nth_value (attr, 0));
#endif

#if 0
	/ * AS does not support ExtendedAddress * /
	set_xml_element (appData, add1, (const xmlChar*) attribute_get_nth_value (attr, 1));
#endif

	/* Street */
	set_xml_element (appData, add2, (const xmlChar*) attribute_get_nth_value (attr, 2));
	/* City */
	set_xml_element (appData, add3, (const xmlChar*) attribute_get_nth_value (attr, 3));
	/* State */
	set_xml_element (appData, add4, (const xmlChar*) attribute_get_nth_value (attr, 4));
	/* Postalcode */
	set_xml_element (appData, add5, (const xmlChar*) attribute_get_nth_value (attr, 5));
	/* Country */
	set_xml_element (appData, add6, (const xmlChar*) attribute_get_nth_value (attr, 6));
}

static void
set_xml_address (xmlNodePtr appData, EVCardAttribute *attr)
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
	int type = attribute_get_type (attr);

	/* HOME and OTHER must be set explicitly, WORK is the default */
	if (type & HOME_MASK) {
		set_xml_address2 (appData, attr,
				  (const xmlChar*) "PostalBox",
				  (const xmlChar*) "ExtendedAddress",
				  (const xmlChar*) EAS_ELEMENT_HOMESTREET,
				  (const xmlChar*) EAS_ELEMENT_HOMECITY,
				  (const xmlChar*) EAS_ELEMENT_HOMESTATE,
				  (const xmlChar*) EAS_ELEMENT_HOMEPOSTALCODE,
				  (const xmlChar*) EAS_ELEMENT_HOMECOUNTRY);
	} else if (type & OTHER_MASK) {
		/* deal with possible other vCard type of addresses:
		 "dom", "intl", "postal", "parcel", "pref" / iana-type / x-name */
		set_xml_address2 (appData, attr,
				  (const xmlChar*) "PostalBox",
				  (const xmlChar*) "ExtendedAddress",
				  (const xmlChar*) EAS_ELEMENT_OTHERSTREET,
				  (const xmlChar*) EAS_ELEMENT_OTHERCITY,
				  (const xmlChar*) EAS_ELEMENT_OTHERSTATE,
				  (const xmlChar*) EAS_ELEMENT_OTHERPOSTALCODE,
				  (const xmlChar*) EAS_ELEMENT_OTHERCOUNTRY);
	} else {
		set_xml_address2 (appData, attr,
				  (const xmlChar*) "PostalBox",
				  (const xmlChar*) "ExtendedAddress",
				  (const xmlChar*) EAS_ELEMENT_BUSINESSSTREET,
				  (const xmlChar*) EAS_ELEMENT_BUSINESSCITY,
				  (const xmlChar*) EAS_ELEMENT_BUSINESSSTATE,
				  (const xmlChar*) EAS_ELEMENT_BUSINESSPOSTALCODE,
				  (const xmlChar*) EAS_ELEMENT_BUSINESSCOUNTRY);
	}
}


static void
set_xml_tel (xmlNodePtr appData, EVCardAttribute *attr)
{
	int type = attribute_get_type (attr);

	/* check most specific types first, ignore unknown types (PREF, ISDN, MODEM, ...) */
	if ((type & WORK_MASK) && (type & FAX_MASK)) {
		if (!is_element_set (appData, EAS_ELEMENT_BUSINESSFAXNUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_BUSINESSFAXNUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if ((type & HOME_MASK) && (type & FAX_MASK)) {
		if (!is_element_set (appData, EAS_ELEMENT_HOMEFAXNUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_HOMEFAXNUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if (type & CELL_MASK) {
		if (!is_element_set (appData, EAS_ELEMENT_MOBILEPHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_MOBILEPHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if (type & CAR_MASK) {
		if (!is_element_set (appData, EAS_ELEMENT_CARPHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_CARPHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if (type & RADIO_MASK) {
		if (!is_element_set (appData, EAS_ELEMENT_RADIOPHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_RADIOPHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if (type & PAGER_MASK) {
		if (!is_element_set (appData, EAS_ELEMENT_PAGER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_PAGER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else if (type & WORK_MASK) {
		/* VOICE is the default which is used if FAX/CELL/... wasn't set explicitly */
		if (!is_element_set (appData, EAS_ELEMENT_BUSINESSPHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_BUSINESSPHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));

		} else if (!is_element_set (appData, EAS_ELEMENT_BUSINESS2PHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_BUSINESS2PHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	} else {
		/* VOICE,HOME is the default */
		if (!is_element_set (appData, EAS_ELEMENT_HOMEPHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_HOMEPHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		} else if (!is_element_set (appData, EAS_ELEMENT_HOME2PHONENUMBER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_HOME2PHONENUMBER,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
		}
	}
}

static void
set_xml_email (xmlNodePtr appData, EVCardAttribute *attr)
{
	if (!is_element_set (appData, EAS_ELEMENT_EMAIL1ADDRESS)) {
		set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_EMAIL1ADDRESS,
				 (const xmlChar*) attribute_get_nth_value (attr, 0));
	} else if (!is_element_set (appData, EAS_ELEMENT_EMAIL2ADDRESS)) {
		set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_EMAIL2ADDRESS,
				 (const xmlChar*) attribute_get_nth_value (attr, 0));
	} else if (!is_element_set (appData, EAS_ELEMENT_EMAIL3ADDRESS)) {
		set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_EMAIL3ADDRESS,
				 (const xmlChar*) attribute_get_nth_value (attr, 0));
	} else
		g_debug ("all three emails already set, dropping remaining one(s)");

}

static void
set_xml_categories (xmlNodePtr appData, EVCardAttribute *attr)
{
	xmlNodePtr categories = NULL;
	GList *values = NULL;
	GList *l = NULL;
	categories = xmlNewChild (appData, NULL, (xmlChar *) EAS_ELEMENT_CATEGORIES, NULL);
	values = e_vcard_attribute_get_values (attr);
	for (l = values; l; l = l->next) {
		set_xml_element (categories, (const xmlChar*) EAS_ELEMENT_CATEGORY,
				 (const xmlChar*) l->data);
	}
}

static void
set_xml_Note_text (xmlNodePtr appData, const char *text)
{
	xmlNodePtr body = NULL;

	body = xmlNewChild (appData, NULL, (xmlChar *)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY, NULL);
	set_xml_element(body, (const xmlChar*)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_TYPE,
			(const xmlChar*) "1");
	set_xml_element(body, (const xmlChar*)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_SIZE,
			(const xmlChar*) "0");
	/* set_xml_element(body, (const xmlChar*)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_TRUNCATED,
	   (const xmlChar*) "0"); */
	set_xml_element(body, (const xmlChar*)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_DATA,
			(const xmlChar*) (const xmlChar*)text);
	/* set_xml_element(body, (const xmlChar*)EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_PREVIEW,
	   (const xmlChar*) "0");*/
}

static void
set_xml_Note (xmlNodePtr appData, EVCardAttribute *attr)
{
	set_xml_Note_text (appData, attribute_get_nth_value(attr, 0));
}

static void
set_xml_contact_date (xmlNodePtr appData, EVCardAttribute *attr, gchar* eas_element)
{
	/* vCard/Evolution defines the date as YYYY-MM-DD[Thh:mm:ssZ|Thh:mm:ss[+-]hh:mm]
	   we need to convert it to ActiveSync YYYY-MM-DDT00:00:00.000Z */
	const gchar* date = attribute_get_nth_value (attr, 0);
	gchar *time = strchr (date, 'T');
	size_t datelen = time ? (time - date) : strlen(date);
	int seconds = 0;
	long value;
	gchar buffer[80];

	/*
	 * parse optional hh:mm:ssZ|Thh:mm:ss[+-]hh:mm:
	 * no error handling, garbage in/garbage out,
	 * time in UTC before midnight not supported
	 * (=> 20101201T00:00:00-01:00 becomes
	 *     20101201T00:00:00.000Z instead of
	 *     decreasing the date)
	 */
	if (time) {
		value = strtol (time + 1, &time, 10);
		seconds += value * 60 * 60;
		if (time && *time == ':') {
			value = strtol (time + 1, &time, 10);
			seconds += value * 60;
			if (time && *time == ':') {
				value = strtol (time + 1, &time, 10);
				seconds += value;
				if (time && (*time == '-' || *time == '+')) {
					/* UTC offset as hour:min */
					int sign = *time == '-' ? -1 : +1;
					value = strtol (time + 1, &time, 10);
					seconds -= sign * value * 60 * 60;
					if (time && (*time == ':')) {
						value = strtol (time + 1, &time, 10);
						seconds -= sign * value * 60;
					}
				}
			}
		}
	}

	/* accept both long (YYYY-MM-DD) and short format for date, produce long version */
	if (datelen > sizeof(buffer) - 40)
		datelen = sizeof(buffer) - 40;
	if (datelen == 4 + 2 + 2)
		sprintf (buffer, "%.4s-%.2s-%.2s",
			 date,
			 date + 4,
			 date + 4 + 2);
	else {
		memcpy (buffer, date, datelen);
		buffer[datelen] = 0;
	}

	/* now format the time */
	if (seconds < 0) {
		/* would have to change the date; instead reset to midnight */
		seconds = 0;
	}
	sprintf (buffer + strlen (buffer), "T%02d:%02d:%02d.000Z",
		 seconds / (60 * 60) % 24,
		 seconds / 60 % 60,
		 seconds % 60);
	set_xml_element (appData, (const xmlChar*) eas_element, (const xmlChar*)buffer);
}

/**
 * Ensure that all XML properties are set. Otherwise removing
 * properties is not possible. It would be nice if this could be
 * limited to properties which exist on the server, but the daemon
 * doesn't track that information.
 */
static void
set_missing_contact_properties (xmlNodePtr appData)
{
	const char *elements[] = {
		EAS_ELEMENT_LASTNAME,
		EAS_ELEMENT_FIRSTNAME,
		EAS_ELEMENT_MIDDLENAME,
		EAS_ELEMENT_SUFFIX,
		EAS_ELEMENT_COMPANYNAME,
		EAS_ELEMENT_DEPARTMENT,
		EAS_ELEMENT_OFFICELOCATION,
		EAS_ELEMENT_BUSINESSFAXNUMBER,
		EAS_ELEMENT_HOMEFAXNUMBER,
		EAS_ELEMENT_MOBILEPHONENUMBER,
		EAS_ELEMENT_CARPHONENUMBER,
		EAS_ELEMENT_RADIOPHONENUMBER,
		EAS_ELEMENT_PAGER,
		EAS_ELEMENT_BUSINESSPHONENUMBER,
		EAS_ELEMENT_BUSINESS2PHONENUMBER,
		EAS_ELEMENT_HOMEPHONENUMBER,
		EAS_ELEMENT_HOME2PHONENUMBER,
		EAS_ELEMENT_EMAIL1ADDRESS,
		EAS_ELEMENT_EMAIL2ADDRESS,
		EAS_ELEMENT_EMAIL3ADDRESS,
		EAS_ELEMENT_CONTACTS2_NICKNAME,
		EAS_ELEMENT_URL,
		EAS_ELEMENT_TITLE,
		EAS_ELEMENT_ROLE,
		EAS_ELEMENT_SPOUSE,
		EAS_ELEMENT_FILEAS,
		EAS_ELEMENT_ASSISTANTNAME,
		EAS_ELEMENT_CONTACTS2_MANAGERNAME,
		EAS_ELEMENT_OFFICELOCATION,
		EAS_ELEMENT_PHOTO,

		/* can be set like a text element */
		EAS_ELEMENT_CATEGORIES,

		NULL
	};
	int i;

	for (i = 0; elements[i]; i++)
		if (!is_element_set (appData, elements[i]))
			set_xml_element (appData, (const xmlChar *)elements[i], (const xmlChar *)"");

	/* special case for body */
	if (!is_element_set (appData, EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY))
		set_xml_Note_text (appData, "");
}

gboolean
eas_con_info_translator_parse_request (xmlDocPtr doc,
				       xmlNodePtr appData,
				       EasItemInfo* contactInfo)
{
	gboolean success = FALSE;
	EVCard* vcard = NULL;
	GList *a = NULL;
	GList *attributes = NULL;

	g_return_val_if_fail (doc != NULL && appData != NULL && contactInfo != NULL, FALSE);
	g_return_val_if_fail (appData->type == XML_ELEMENT_NODE, FALSE);
	g_return_val_if_fail (appData->name != NULL, FALSE);
	g_return_val_if_fail (g_strcmp0 ( (char*) (appData->name), EAS_ELEMENT_APPLICATIONDATA) == 0, FALSE);

	g_debug ("eas_con_info_translator_parse_request ++");

#if !GLIB_CHECK_VERSION(2,36,0)
	g_type_init();
#endif

	vcard = e_vcard_new_from_string (contactInfo->data);
	g_return_val_if_fail (vcard != NULL, FALSE);

	/* e_vcard_dump_structure (vcard); */ /* Debug only*/

	attributes = e_vcard_get_attributes (vcard);

	for (a = attributes; a; a = a->next) {
		const char *name = NULL;
		EVCardAttribute *attr = a->data;
		success = TRUE;
		name = e_vcard_attribute_get_name (attr);

		/* g_debug("e_vcard_attribute_get_name=%s", name); */

		if (!strcmp (name, "BEGIN"))
			continue;

		if (!strcmp (name, EVC_VERSION))
			continue;


		/* process attributes that have no param */

		/* Name */
		if (!strcmp (name, EVC_N)) {
			set_xml_name (appData, attr);
			continue;
		}

		/* NICKNAME */
		/* TODO: -NickName (Contacts2)-> does not show in Exchange Outlook
		 		 -Alias -> causes AS malformed item error:
		  				MS-ASWBXML and wbxml (in wbxml_tables.c) indicate that this
						item is not supported when the MS-ASProtocolVersion header is set to 12.1 */
		if (!strcmp (name, EVC_NICKNAME)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_CONTACTS2_NICKNAME,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* Company */
		if (!strcmp (name, EVC_ORG)) {
			set_xml_org (appData, attr);
			continue;
		}

		/* Url */
		if (!strcmp (name, EVC_URL)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_URL,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));

			continue;
		}

		/* Birthday */
		if (!strcmp (name, EVC_BDAY)) {
			set_xml_contact_date (appData, attr, (gchar*) EAS_ELEMENT_BIRTHDAY);
			continue;
		}

		/* Title */
		if (!strcmp (name, EVC_TITLE)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_TITLE,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* Role */
		if (!strcmp (name, EVC_ROLE)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_ROLE,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* Spouse - vCard does not support Spouse so we use X-EVOLUTION-SPOUSE*/
		if (!strcmp (name, EVC_X_SPOUSE)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_SPOUSE,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* FileAs - vCard does not support FileAs so we use X-EVOLUTION-FILE-AS*/
		if (!strcmp (name, EVC_X_FILE_AS)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_FILEAS,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* AssistantName - vCard does not support Assistant so we use X-EVOLUTION-ASSISTANT*/
		if (!strcmp (name, EVC_X_ASSISTANT)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_ASSISTANTNAME,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* ManagerName - vCard does not support Manager so we use X-EVOLUTION-MANAGER*/
		if (!strcmp (name, EVC_X_MANAGER)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_CONTACTS2_MANAGERNAME,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* Anniversary - vCard does not support Anniversary so we use X-EVOLUTION-ANNIVERSARY*/
		if (!strcmp (name, EVC_X_ANNIVERSARY)) {
			set_xml_contact_date (appData, attr, (gchar*) EAS_ELEMENT_ANNIVERSARY);
			continue;
		}

		/* Note */
		if (!strcmp (name, EVC_NOTE)) {
			set_xml_Note (appData, attr);
			continue;
		}

		/* Photo (AS: name is "Picture") */
		/* Evolution saves Photo as base64 encoded so there is no need to encode*/
		if (!strcmp (name, EVC_PHOTO)) {
			set_xml_element (appData, (const xmlChar*) EAS_ELEMENT_PHOTO,
					 (const xmlChar*) attribute_get_nth_value (attr, 0));
			continue;
		}

		/* Categories  */
		if (!strcmp (name, (const char *) EVC_CATEGORIES)) {
			set_xml_categories (appData, attr);
			continue;
		}

		/* EMail */
		if (!strcmp (name, EVC_EMAIL)) {
			set_xml_email (appData, attr);
			continue;
		}

		/* Telephone */
		if (!strcmp (name, EVC_TEL)) {
			set_xml_tel (appData, attr);
			continue;
		}

		if (!strcmp (name, EVC_ADR)) {
			set_xml_address (appData, attr);
			continue;
		}

#if 0
			/* ActiveSync does not support the following vCard fields: */

			/* FullName */
			if (!strcmp (name, EVC_FN)) {
				continue;
			}

			/* Uid */
			if (!strcmp (name, EVC_UID)) {
				continue;
			}

			/* Address Labeling */
			if (!strcmp (name, EVC_LABEL)) {
				continue;
			}

			/* Mailer */
			if (!strcmp (name, EVC_MAILER)) {
				continue;
			}

			/* Timezone */
			if (!strcmp (name, "TZ")) {
				continue;
			}

			/* Location */
			if (!strcmp (name, EVC_GEO)) {
				continue;
			}

			/* Logo */
			if (!strcmp (name, EVC_LOGO)) {
				continue;
			}

			/* Revision */
			if (!strcmp (name, EVC_REV)) {
				continue;
			}

			/* Sound */
			if (!strcmp (name, "SOUND")) {
				continue;
			}

			/* Public Key */
			if (!strcmp (name, EVC_KEY)) {
				continue;
			}
#endif
	}


	g_object_unref (vcard);

	set_missing_contact_properties (appData);

	if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 4)) {
		xmlChar* dump_buffer = NULL;
		int dump_buffer_size = 0;
		xmlIndentTreeOutput = 1;
		xmlDocDumpFormatMemory (doc, &dump_buffer, &dump_buffer_size, 1);
		g_debug ("XML DOCUMENT DUMPED:\n%s", dump_buffer);
		xmlFree (dump_buffer);
	}

	g_debug ("eas_con_info_translator_parse_request --");
	return success;
}
