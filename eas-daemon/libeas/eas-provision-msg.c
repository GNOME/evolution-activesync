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

#include "eas-provision-msg.h"
#include "eas-connection-errors.h"
#include "eas-provision-list.h"


struct _EasProvisionMsgPrivate {
	gchar* policy_key;
	gchar* policy_status;
	EasProvisionList* provision_list;
};

#define EAS_PROVISION_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_PROVISION_MSG, EasProvisionMsgPrivate))



G_DEFINE_TYPE (EasProvisionMsg, eas_provision_msg, EAS_TYPE_MSG_BASE);

static void
eas_provision_msg_init (EasProvisionMsg *object)
{
	EasProvisionMsgPrivate *priv;
	g_debug ("eas_provision_msg_init++");

	object->priv = priv = EAS_PROVISION_MSG_PRIVATE (object);

	priv->policy_key = NULL;
	priv->policy_status = NULL;

	g_debug ("eas_provision_msg_init--");
}

static void
eas_provision_msg_finalize (GObject *object)
{
	EasProvisionMsg *msg = (EasProvisionMsg *) object;
	EasProvisionMsgPrivate *priv = msg->priv;

	g_debug ("eas_provision_msg_finalize++");

	// g_free ignores NULL
	g_free (priv->policy_key);
	g_free (priv->policy_status);

	G_OBJECT_CLASS (eas_provision_msg_parent_class)->finalize (object);
	g_debug ("eas_provision_msg_finalize--");
}

static void
eas_provision_msg_class_init (EasProvisionMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_provision_msg_class_init++");

	g_type_class_add_private (klass, sizeof (EasProvisionMsgPrivate));

	object_class->finalize = eas_provision_msg_finalize;

	g_debug ("eas_provision_msg_class_init--");
}


xmlDoc*
eas_provision_msg_build_message (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	xmlNode *node = NULL,
		 *child = NULL,
		  *grandchild = NULL;
	xmlNs *ns = NULL;

	g_debug ("eas_provision_msg_build_message++");

	doc = xmlNewDoc ( (xmlChar *) "1.0");
	node = xmlNewDocNode (doc, NULL, (xmlChar*) "Provision", NULL);
	xmlDocSetRootElement (doc, node);

	xmlCreateIntSubset (doc,
			    (xmlChar*) "ActiveSync",
			    (xmlChar*) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar*) "http://www.microsoft.com/");

	ns = xmlNewNs (node, (xmlChar *) "Provision:", NULL);
	xmlNewNsProp (node, ns, (xmlChar*) "xmlns:settings", (xmlChar*) "Settings:");
	child = xmlNewChild (node, ns, (xmlChar *) "Policies", NULL);
	grandchild = xmlNewChild (child, ns, (xmlChar*) "Policy", NULL);
	xmlNewChild (grandchild, ns, (xmlChar*) "PolicyType", (xmlChar*) "MS-EAS-Provisioning-WBXML");

	if (priv->policy_key) {
		xmlNewChild (grandchild, ns, (xmlChar*) "PolicyKey", (xmlChar*) priv->policy_key);
	}
	if (priv->policy_status) {
		xmlNewChild (grandchild, ns, (xmlChar*) "Status", (xmlChar*) priv->policy_status);
	}

	g_debug ("eas_provision_msg_build_message--");

	return doc;
}

/*
translates from eas provision status code to GError
*/
/*static void set_provision_status_error (guint provision_status, GError **error)
{
	switch (provision_status) {
	case 2: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_PROTOCOLERROR,
			     ("Provisioning error: protocol error"));
	}
	break;
	case 3: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_GENERALSERVERERROR,
			     ("Provisioning error: general server error"));
	}
	break;
	case 4: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_DEVICE_EXTERNALLY_MANAGED,
			     ("Provisioning error: device externally managed"));
	}
	break;
	default: {
		g_warning ("Unrecognised provisioning error");
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,
			     ("Unrecognised provisioning error"));
	}
	}
}*/

/*
translates from eas policy status code to GError
*/
static void set_policy_status_error (guint policy_status, GError **error)
{
	switch (policy_status) {
	case 2: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_NOCLIENTPOLICYEXISTS,
			     ("Provisioning error: No policy for this client"));
	}
	break;
	case 3: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_UNKNOWNPOLICYTYPE,
			     ("Provisioning error: unknown policy type value"));
	}
	break;
	case 4: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_CORRUPTSERVERPOLICYDATA,
			     ("Provisioning error: policy data on the server is corrupted (possibly tampered with)"));
	}
	break;
	case 5: {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_ACKINGWRONGPOLICYKEY,
			     ("Provisioning error: client is acknowledging the wrong policy key"));
	}
	default: {
		g_warning ("Unrecognised provisioning error");
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,
			     ("unrecognised provisioning error"));
	}
	break;
	}
}

gboolean
eas_provision_msg_parse_response (EasProvisionMsg* self, xmlDoc* doc, GError** error)
{
	gboolean success = FALSE;
	
	EasProvisionMsgPrivate *priv = self->priv;
	xmlNode *node1 = NULL;

	g_debug ("eas_provision_msg_parse_response++");

	/* Initialise priv members correctly */
	g_free (priv->policy_key);
	priv->policy_key = NULL;
	
	g_free (priv->policy_status);
	priv->policy_status = NULL;

	if (priv->provision_list)
	{
		g_object_unref(priv->provision_list);
		priv->provision_list = NULL;
	}
	priv->provision_list = eas_provision_list_new();

	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_CONNECTION_ERROR_BADARG,
			     ("bad argument"));
		return FALSE;
	}

	/* Check root node == <Provision> */
	node1 = xmlDocGetRootElement (doc);
	if (g_strcmp0 ( (char *) node1->name, "Provision")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_COMMON_STATUS_INVALIDXML,
			     ("Failed to find <Provision> element"));
		return FALSE;
	}

	/* Navigate to Provision > Policies */
	for (node1 = node1->children; node1; node1 = node1->next) {
		if (node1->type == XML_ELEMENT_NODE && !g_strcmp0((char*)node1->name, "Policies")) {
			/* Navigate to Provision > Policies > Policy */
			xmlNode* node2;
			for (node2 = node1->children; node2; node2 = node2->next) {
				if (node2->type == XML_ELEMENT_NODE && !g_strcmp0((char*)node2->name, "Policy")) {
					/* Find Status & PolicyKey */
					xmlNode* node3;
					for (node3 = node2->children; node3; node3 = node3->next) {
						const char* name = (char*)node3->name;
						if (node3->type != XML_ELEMENT_NODE)
							continue;

						if (!g_strcmp0(name, "Status")) {
							// TODO: check this makes a new copy
							priv->policy_status = (gchar*)xmlNodeGetContent(node3);
							g_debug ("Provisioned Policy Status:[%s]", priv->policy_status);
						}
						else if (!g_strcmp0(name, "PolicyKey")) {
							// TODO: check this makes a new copy
							priv->policy_key = (gchar*)xmlNodeGetContent(node3);
							g_debug ("Provisioned Policy Key:[%s]", priv->policy_key);
						}
						else if (!g_strcmp0(name, "Data")) {
							xmlNode* node4;
							g_debug("Parsing <Data> node...");
							for (node4 = node3->children; node4; node4 = node4->next) {
								if (node4->type == XML_ELEMENT_NODE && !g_strcmp0((char*)node4->name, "EASProvisionDoc")) {
									xmlNode* node5;
									g_debug("Parsing <EasProvisionDoc> node...");
									/* FINALLY! We've made it to the provisioning requirements */ 
									for (node5 = node4->children; node5; node5 = node5->next) {
										const char* name = (char*)node5->name;
										if (node5->type != XML_ELEMENT_NODE)
											continue;

										g_debug("Parsing provision list nodes...");
						
										if (!g_strcmp0(name, "DevicePasswordEnabled")) {
											priv->provision_list->DevicePasswordEnabled = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AlphanumericDevicePasswordRequired")) {
											priv->provision_list->AlphaNumericDevicePasswordRequired = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "PasswordRecoveryEnabled")) {
											priv->provision_list->PasswordRecoveryEnabled = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequiresStorageCardEncryption")) {
											priv->provision_list->RequireStorageCardEncryption = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AttachmentsEnabled")) {
											priv->provision_list->AttachmentsEnabled = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MinDevicePasswordLength")) {
											priv->provision_list->MinDevicePasswordLength = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxInactivityTimeDeviceLock")) {
											priv->provision_list->MaxInactivityTimeDeviceLock = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxDevicePasswordFailedAttempts")) {
											priv->provision_list->MaxDevicePasswordFailedAttempts = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxAttachmentSize")) {
											priv->provision_list->MaxAttachmentSize = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowSimpleDevicePassword")) {
											priv->provision_list->AllowSimpleDevicePassword = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "DevicePasswordExpiration")) {
											priv->provision_list->DevicePasswordExpiration = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "DevicePasswordHistory")) {
											priv->provision_list->DevicePasswordHistory = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowStorageCard")) {
											priv->provision_list->AllowStorageCard = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowCamera")) {
											priv->provision_list->AllowCamera = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireDeviceEncryption")) {
											priv->provision_list->RequireDeviceEncryption = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowUnsignedApplications")) {
											priv->provision_list->AllowUnsignedApplications = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowUnsignedInstallationPackages")) {
											priv->provision_list->AllowUnsignedInstallationPackages = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MinDevicePasswordComplexCharacters")) {
											priv->provision_list->MinDevicePasswordComplexCharacters = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowWifi")) {
											priv->provision_list->AllowWifi = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowTextMessaging")) {
											priv->provision_list->AllowTextMessaging = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowPOPIMAPEmail")) {
											priv->provision_list->AllowPOPIMAPEmail = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowBluetooth")) {
											priv->provision_list->AllowBluetooth = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowIrDA")) {
											priv->provision_list->AllowIrDA = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireManualSyncWhenRoaming")) {
											priv->provision_list->RequireManualSyncWhenRoaming = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowDesktopSync")) {
											priv->provision_list->AllowDesktopSync = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxCalendarAgeFilter")) {
											priv->provision_list->MaxCalendarAgeFilter = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowHTMLEmail")) {
											priv->provision_list->AllowHTMLEmail = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxEmailAgeFilter")) {
											priv->provision_list->MaxEmailAgeFilter = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxEmailBodyTruncationSize")) {
											priv->provision_list->MaxEmailBodyTruncationSize = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "MaxEmailHTMLBodyTruncationSize")) {
											priv->provision_list->MaxEmailHTMLBodyTruncationSize = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireSignedSMIMEMessages")) {
											priv->provision_list->RequireSignedSMIMEMessages = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireEncryptedSMIMEMessages")) {
											priv->provision_list->RequireEncryptedSMIMEMessages = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireSignedSMIMEAlgorithm")) {
											priv->provision_list->RequireSignedSMIMEAlgorithm = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "RequireEncryptionSMIMEAlgorithm")) {
											priv->provision_list->RequireEncryptionSMIMEAlgorithm = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowSMIMEEncryptionAlgorithmNegotiation")) {
											priv->provision_list->AllowSMIMEEncryptionAlgorithmNegotiation = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowSMIMESoftCerts")) {
											priv->provision_list->AllowSMIMESoftCerts = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowBrowser")) {
											priv->provision_list->AllowBrowser = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowConsumerEmail")) {
											priv->provision_list->AllowConsumerEmail = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowRemoteDesktop")) {
											priv->provision_list->AllowRemoteDesktop = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "AllowInternetSharing")) {
											priv->provision_list->AllowInternetSharing = (gchar*)xmlNodeGetContent(node5);
										}
										else if (!g_strcmp0(name, "UnapprovedInROMApplicationList")) {
											// TODO
										}
										else if (!g_strcmp0(name, "ApprovedApplicationList")) {
											// TODO
										}
									}
									g_debug("End of for loop...");
								}
							}							
							g_debug("End of for loop...");
						}
					}					
					g_debug("End of for loop...");
				}
			}
			g_debug("End of for loop...");
		}
	}
	g_debug("End of for loop...");

	/* Check for errors */
	if (!priv->policy_status) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_COMMON_STATUS_INVALIDXML,
			     ("Failed to find <Status> element"));
		success = FALSE;
	}
	else if (!priv->policy_key) {
		g_set_error (error, EAS_CONNECTION_ERROR,
			     EAS_COMMON_STATUS_INVALIDXML,
			     ("Failed to find <PolicyKey> element"));
		success = FALSE;
	}
	else {
		/* Check the status code was good */
		guint policy_status_num = atoi(priv->policy_status);
		if (policy_status_num != 1) { // not success
			set_policy_status_error (policy_status_num, error);
			success = FALSE;
		}
		else {
			success = TRUE;
		}
	}

	g_debug ("eas_provision_msg_parse_response--");

	return success;
}

EasProvisionMsg*
eas_provision_msg_new (void)
{
	EasProvisionMsg* msg = NULL;

	g_debug ("eas_provision_msg_new++");

	msg = g_object_new (EAS_TYPE_PROVISION_MSG, NULL);

	g_debug ("eas_provision_msg_new--");

	return msg;
}

gchar*
eas_provision_msg_get_policy_key (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_debug ("eas_provision_msg_get_policy_key+-");

	return priv->policy_key;
}

gchar*
eas_provision_msg_get_policy_status (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate *priv = self->priv;
	g_debug ("eas_provision_msg_get_policy_status+-");
	return priv->policy_status;
}

EasProvisionList*
eas_provision_msg_get_provision_list (EasProvisionMsg* self)
{
	EasProvisionMsgPrivate* priv = self->priv;
	g_debug("eas_provision_msg_get_provision_list+-");
	return priv->provision_list;
}

void
eas_provision_msg_set_policy_key (EasProvisionMsg* self, const gchar* policyKey)
{
	EasProvisionMsgPrivate *priv = self->priv;

	g_debug ("eas_provision_msg_set_policy_key++");

	// g_xxx functions can handle NULL
	g_free (priv->policy_key);
	priv->policy_key = g_strdup (policyKey);

	g_debug ("eas_provision_msg_set_policy_key--");
}

void
eas_provision_msg_set_policy_status (EasProvisionMsg* self, const gchar* policyStatus)
{
	EasProvisionMsgPrivate *priv = self->priv;

	g_debug ("eas_provision_msg_set_policy_status++");

	// g_xxx functions can handle NULL
	g_free (priv->policy_status);
	priv->policy_status = g_strdup (policyStatus);

	g_debug ("eas_provision_msg_set_policy_status--");
}

