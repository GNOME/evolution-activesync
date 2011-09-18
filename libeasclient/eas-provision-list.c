/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 *
 */

#include "eas-provision-list.h"

const gchar *list_separator = "\n";
const gchar *app_separator = ",";

#define STRING_LIST_SIZE 43


G_DEFINE_TYPE (EasProvisionList, eas_provision_list, G_TYPE_OBJECT);

static void
eas_provision_list_init (EasProvisionList *object)
{
	object->DevicePasswordEnabled  = NULL;
	object->AlphaNumericDevicePasswordRequired  = NULL;
	object->PasswordRecoveryEnabled  = NULL;
	object->RequireStorageCardEncryption  = NULL;
	object->AttachmentsEnabled  = NULL;
	object->MinDevicePasswordLength  = NULL;
	object->MaxInactivityTimeDeviceLock  = NULL;
	object->MaxDevicePasswordFailedAttempts  = NULL;
	object->MaxAttachmentSize  = NULL;
	object->AllowSimpleDevicePassword  = NULL;
	object->DevicePasswordExpiration  = NULL;
	object->DevicePasswordHistory  = NULL;
	object->AllowStorageCard  = NULL;
	object->AllowCamera  = NULL;
	object->RequireDeviceEncryption  = NULL;
	object->AllowUnsignedApplications  = NULL;
	object->AllowUnsignedInstallationPackages  = NULL;
	object->MinDevicePasswordComplexCharacters = NULL;
	object->AllowWifi  = NULL;
	object->AllowTextMessaging  = NULL;
	object->AllowPOPIMAPEmail  = NULL;
	object->AllowBluetooth  = NULL;
	object->AllowIrDA  = NULL;
	object->RequireManualSyncWhenRoaming  = NULL;
	object->AllowDesktopSync  = NULL;
	object->MaxCalendarAgeFilter  = NULL;
	object->AllowHTMLEmail  = NULL;
	object->MaxEmailAgeFilter  = NULL;
	object->MaxEmailBodyTruncationSize  = NULL;
	object->MaxEmailHTMLBodyTruncationSize  = NULL;
	object->RequireSignedSMIMEMessages  = NULL;
	object->RequireEncryptedSMIMEMessages  = NULL;
	object->RequireSignedSMIMEAlgorithm  = NULL;
	object->RequireEncryptionSMIMEAlgorithm  = NULL;
	object->AllowSMIMEEncryptionAlgorithmNegotiation  = NULL;
	object->AllowSMIMESoftCerts  = NULL;
	object->AllowBrowser  = NULL;
	object->AllowConsumerEmail  = NULL;
	object->AllowRemoteDesktop  = NULL;
	object->AllowInternetSharing  = NULL;
}

static void
eas_provision_list_finalize (GObject *object)
{
	EasProvisionList *self = (EasProvisionList *) object;
	g_free (self->DevicePasswordEnabled);
	g_free (self->AlphaNumericDevicePasswordRequired);
	g_free (self->PasswordRecoveryEnabled);
	g_free (self->RequireStorageCardEncryption);
	g_free (self->AttachmentsEnabled);
	g_free (self->MinDevicePasswordLength);
	g_free (self->MaxInactivityTimeDeviceLock);
	g_free (self->MaxDevicePasswordFailedAttempts);
	g_free (self->MaxAttachmentSize);
	g_free (self->AllowSimpleDevicePassword);
	g_free (self->DevicePasswordExpiration);
	g_free (self->DevicePasswordHistory);
	g_free (self->AllowStorageCard);
	g_free (self->AllowCamera);
	g_free (self->RequireDeviceEncryption);
	g_free (self->AllowUnsignedApplications);
	g_free (self->AllowUnsignedInstallationPackages);
	g_free (self->MinDevicePasswordComplexCharacters);
	g_free (self->AllowWifi);
	g_free (self->AllowTextMessaging);
	g_free (self->AllowPOPIMAPEmail);
	g_free (self->AllowBluetooth);
	g_free (self->AllowIrDA);
	g_free (self->RequireManualSyncWhenRoaming);
	g_free (self->AllowDesktopSync);
	g_free (self->MaxCalendarAgeFilter);
	g_free (self->AllowHTMLEmail);
	g_free (self->MaxEmailAgeFilter);
	g_free (self->MaxEmailBodyTruncationSize);
	g_free (self->MaxEmailHTMLBodyTruncationSize);
	g_free (self->RequireSignedSMIMEMessages);
	g_free (self->RequireEncryptedSMIMEMessages);
	g_free (self->RequireSignedSMIMEAlgorithm);
	g_free (self->RequireEncryptionSMIMEAlgorithm);
	g_free (self->AllowSMIMEEncryptionAlgorithmNegotiation);
	g_free (self->AllowSMIMESoftCerts);
	g_free (self->AllowBrowser);
	g_free (self->AllowConsumerEmail);
	g_free (self->AllowRemoteDesktop);
	g_free (self->AllowInternetSharing);

	G_OBJECT_CLASS (eas_provision_list_parent_class)->finalize (object);
}

static void
eas_provision_list_class_init (EasProvisionListClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_provision_list_finalize;
}


EasProvisionList*
eas_provision_list_new()
{
	EasProvisionList* self = g_object_new (EAS_TYPE_PROVISION_LIST, NULL);
	return self;
}


gboolean
eas_provision_list_serialise (EasProvisionList* list, gchar **result)
{
	gboolean ret = TRUE;
	gchar* UnapprovedList = NULL;
	gchar* ApprovedList = NULL;
	GSList *iterator = NULL;
	gchar *empty = (gchar *) "";
	gchar *strings[STRING_LIST_SIZE] = {
		list->DevicePasswordEnabled ? : empty,
		list->AlphaNumericDevicePasswordRequired ? : empty,
		list->PasswordRecoveryEnabled ? : empty,
		list->RequireStorageCardEncryption ? : empty,
		list->AttachmentsEnabled ? : empty,
		list->MinDevicePasswordLength ? : empty,
		list->MaxInactivityTimeDeviceLock ? : empty,
		list->MaxDevicePasswordFailedAttempts ? : empty,
		list->MaxAttachmentSize ? : empty,
		list->AllowSimpleDevicePassword ? : empty,
		list->DevicePasswordExpiration ? : empty,
		list->DevicePasswordHistory ? : empty,
		list->AllowStorageCard ? : empty,
		list->AllowCamera ? : empty,
		list->RequireDeviceEncryption ? : empty,
		list->AllowUnsignedApplications ? : empty,
		list->AllowUnsignedInstallationPackages ? : empty,
		list->MinDevicePasswordComplexCharacters ? : empty,
		list->AllowWifi ? : empty,
		list->AllowTextMessaging ? : empty,
		list->AllowPOPIMAPEmail ? : empty,
		list->AllowBluetooth ? : empty,
		list->AllowIrDA ? : empty,
		list->RequireManualSyncWhenRoaming ? : empty,
		list->AllowDesktopSync ? : empty,
		list->MaxCalendarAgeFilter ? : empty,
		list->AllowHTMLEmail ? : empty,
		list->MaxEmailAgeFilter ? : empty,
		list->MaxEmailBodyTruncationSize ? : empty,
		list->MaxEmailHTMLBodyTruncationSize ? : empty,
		list->RequireSignedSMIMEMessages ? : empty,
		list->RequireEncryptedSMIMEMessages ? : empty,
		list->RequireSignedSMIMEAlgorithm ? : empty,
		list->RequireEncryptionSMIMEAlgorithm ? : empty,
		list->AllowSMIMEEncryptionAlgorithmNegotiation ? : empty,
		list->AllowSMIMESoftCerts ? : empty,
		list->AllowBrowser ? : empty,
		list->AllowConsumerEmail ? : empty,
		list->AllowRemoteDesktop ? : empty,
		list->AllowInternetSharing ? : empty,
		empty,
		empty,
		NULL
	};

	iterator = g_slist_next (list->UnapprovedInROMApplicationList);
	while (iterator) {
		gchar* temp = g_strdup (UnapprovedList);
		g_free (UnapprovedList);
		UnapprovedList = g_strconcat (temp, app_separator, (gchar*) iterator->data, NULL);
		g_free (temp);
		iterator = g_slist_next (list->UnapprovedInROMApplicationList);
	}
	if (UnapprovedList)
		strings[STRING_LIST_SIZE - 3] = UnapprovedList;


	iterator = g_slist_next (list->ApprovedApplicationList);
	while (iterator) {
		gchar* temp = g_strdup (ApprovedList);
		g_free (ApprovedList);
		ApprovedList = g_strconcat (temp, app_separator, (gchar*) iterator->data, NULL);
		g_free (temp);
		iterator = g_slist_next (list->ApprovedApplicationList);
	}
	if (ApprovedList)
		strings[STRING_LIST_SIZE - 3] = ApprovedList;

	*result = g_strjoinv (list_separator, strings);

	if (!*result) {
		ret = FALSE;
	}

	return ret;

}


// populate the folder object from a null terminated string eg ",1,,,".
gboolean
eas_provision_list_deserialise (EasProvisionList* list, const gchar *data)
{
	gchar **strv;
	gchar **strx;

	gint i = 0;

	gchar* UnapprovedList = NULL;
	gchar* ApprovedList = NULL;

	g_assert (list);
	g_assert (data);

	strv = g_strsplit (data, list_separator, 0);
	if (!strv || g_strv_length (strv) != (STRING_LIST_SIZE - 1)) {
		g_warning ("Received invalid eas_provision_list: '%s', %d", data, g_strv_length (strv));
		g_strfreev (strv);
		return FALSE;
	}

	i = 0;
	list->DevicePasswordEnabled = strv[i++];
	list->AlphaNumericDevicePasswordRequired = strv[i++];
	list->PasswordRecoveryEnabled = strv[i++];
	list->RequireStorageCardEncryption = strv[i++];
	list->AttachmentsEnabled = strv[i++];
	list->MinDevicePasswordLength = strv[i++];
	list->MaxInactivityTimeDeviceLock = strv[i++];
	list->MaxDevicePasswordFailedAttempts = strv[i++];
	list->MaxAttachmentSize = strv[i++];
	list->AllowSimpleDevicePassword = strv[i++];
	list->DevicePasswordExpiration = strv[i++];
	list->DevicePasswordHistory = strv[i++];
	list->AllowStorageCard = strv[i++];
	list->AllowCamera = strv[i++];
	list->RequireDeviceEncryption = strv[i++];
	list->AllowUnsignedApplications = strv[i++];
	list->AllowUnsignedInstallationPackages = strv[i++];
	list->MinDevicePasswordComplexCharacters = strv[i++];
	list->AllowWifi = strv[i++];
	list->AllowTextMessaging = strv[i++];
	list->AllowPOPIMAPEmail = strv[i++];
	list->AllowBluetooth = strv[i++];
	list->AllowIrDA = strv[i++];
	list->RequireManualSyncWhenRoaming = strv[i++];
	list->AllowDesktopSync = strv[i++];
	list->MaxCalendarAgeFilter = strv[i++];
	list->AllowHTMLEmail = strv[i++];
	list->MaxEmailAgeFilter = strv[i++];
	list->MaxEmailBodyTruncationSize = strv[i++];
	list->MaxEmailHTMLBodyTruncationSize = strv[i++];
	list->RequireSignedSMIMEMessages = strv[i++];
	list->RequireEncryptedSMIMEMessages = strv[i++];
	list->RequireSignedSMIMEAlgorithm = strv[i++];
	list->RequireEncryptionSMIMEAlgorithm = strv[i++];
	list->AllowSMIMEEncryptionAlgorithmNegotiation = strv[i++];
	list->AllowSMIMESoftCerts = strv[i++];
	list->AllowBrowser = strv[i++];
	list->AllowConsumerEmail = strv[i++];
	list->AllowRemoteDesktop = strv[i++];
	list->AllowInternetSharing = strv[i++];
	UnapprovedList = strv[i++];
	ApprovedList  = strv[i++];

	strx = g_strsplit (UnapprovedList, app_separator, 0);
	for (i = 0; i < g_strv_length (strx); i++) {
		list->UnapprovedInROMApplicationList = g_slist_append (list->UnapprovedInROMApplicationList, strx[i]);
	}

	strx = g_strsplit (ApprovedList, app_separator, 0);
	for (i = 0; i < g_strv_length (strx); i++) {
		list->ApprovedApplicationList = g_slist_append (list->ApprovedApplicationList, strx[i]);
	}

	/* We don't use g_strfreev() because we actually stole most of the
	   strings. So free the approved string and the unapproved but not the rest. */
	g_free (strv[STRING_LIST_SIZE - 2]);
	g_free (strv[STRING_LIST_SIZE - 1]);
	return TRUE;
}

