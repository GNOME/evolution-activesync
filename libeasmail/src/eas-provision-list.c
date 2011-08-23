/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-provision-list.h"

const gchar *list_separator = "\n";
const gchar *app_separator = ",";


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

gboolean
eas_provision_list_serialise (EasProvisionList* list, gchar **result)
{
	gboolean ret = TRUE;
	gchar* UnapprovedList = NULL;
	gchar* ApprovedList = NULL;
	GSList *iterator = NULL;
	gchar *strings[41] ={list->DevicePasswordEnabled,
						  list->AlphaNumericDevicePasswordRequired,
						  list->PasswordRecoveryEnabled,
						  list->RequireStorageCardEncryption,
						  list->AttachmentsEnabled,
						  list->MinDevicePasswordLength,
						  list->MaxInactivityTimeDeviceLock,
						  list->MaxDevicePasswordFailedAttempts,
						  list->MaxAttachmentSize,
						  list->AllowSimpleDevicePassword,
						  list->DevicePasswordExpiration,
						  list->DevicePasswordHistory,
						  list->AllowStorageCard,
						  list->AllowCamera,
						  list->RequireDeviceEncryption,
						  list->AllowUnsignedApplications,
						  list->AllowUnsignedInstallationPackages,
						  list->AllowWifi,
						  list->AllowTextMessaging,
						  list->AllowPOPIMAPEmail,
						  list->AllowBluetooth,
						  list->AllowIrDA,
						  list->RequireManualSyncWhenRoaming,
						  list->AllowDesktopSync,
						  list->MaxCalendarAgeFilter,
						  list->AllowHTMLEmail,
						  list->MaxEmailAgeFilter,
						  list->MaxEmailBodyTruncationSize,
						  list->MaxEmailHTMLBodyTruncationSize,
						  list->RequireSignedSMIMEMessages,
						  list->RequireEncryptedSMIMEMessages,
						  list->RequireSignedSMIMEAlgorithm,
						  list->RequireEncryptionSMIMEAlgorithm,
						  list->AllowSMIMEEncryptionAlgorithmNegotiation,
						  list->AllowSMIMESoftCerts,
						  list->AllowBrowser,
						  list->AllowConsumerEmail,
						  list->AllowRemoteDesktop,
						  list->AllowInternetSharing,
						  UnapprovedList,
						  ApprovedList};
	iterator = g_slist_next(list->UnapprovedInROMApplicationList);
	while(iterator)
	{
		gchar* temp = g_strdup(UnapprovedList);
		g_free(UnapprovedList);
		UnapprovedList = g_strconcat(temp, app_separator,(gchar*)iterator->data, NULL );
		g_free(temp);
		iterator = g_slist_next(list->UnapprovedInROMApplicationList);
	}
	iterator = g_slist_next(list->ApprovedApplicationList);
	while(iterator)
	{
		gchar* temp = g_strdup(ApprovedList);
		g_free(ApprovedList);
		ApprovedList = g_strconcat(temp, app_separator,(gchar*)iterator->data, NULL );
		g_free(temp);
		iterator = g_slist_next(list->ApprovedApplicationList);
	}

	
	*result = g_strjoinv (list_separator, strings);

	if (!*result) {
		ret = FALSE;
	}

	return (*result ? TRUE : FALSE);
	
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
	if (!strv || g_strv_length (strv) != 41) {
		g_warning ("Received invalid eas_provision_list: '%s'", data);
		g_strfreev (strv);
		return FALSE;
	}

	list->DevicePasswordEnabled = strv[0];
	list->AlphaNumericDevicePasswordRequired = strv[1];
	list->PasswordRecoveryEnabled = strv[2];
	list->RequireStorageCardEncryption = strv[3];
	list->AttachmentsEnabled = strv[4];
	list->MinDevicePasswordLength = strv[5];
	list->MaxInactivityTimeDeviceLock = strv[6];
	list->MaxDevicePasswordFailedAttempts = strv[7];
	list->MaxAttachmentSize = strv[8];
	list->AllowSimpleDevicePassword = strv[9];
	list->DevicePasswordExpiration = strv[10];
	list->DevicePasswordHistory = strv[11];
	list->AllowStorageCard = strv[12];
	list->AllowCamera = strv[13];
	list->RequireDeviceEncryption = strv[14];
	list->AllowUnsignedApplications = strv[15];
	list->AllowUnsignedInstallationPackages = strv[16];
	list->AllowWifi = strv[17];
	list->AllowTextMessaging = strv[18];
	list->AllowPOPIMAPEmail = strv[19];
	list->AllowBluetooth = strv[20];
	list->AllowIrDA = strv[21];
	list->RequireManualSyncWhenRoaming = strv[22];
	list->AllowDesktopSync = strv[23];
	list->MaxCalendarAgeFilter = strv[24];
	list->AllowHTMLEmail = strv[25];
	list->MaxEmailAgeFilter = strv[26];
	list->MaxEmailBodyTruncationSize = strv[27];
	list->MaxEmailHTMLBodyTruncationSize = strv[28];
	list->RequireSignedSMIMEMessages = strv[29];
	list->RequireEncryptedSMIMEMessages = strv[30];
	list->RequireSignedSMIMEAlgorithm = strv[31];
	list->RequireEncryptionSMIMEAlgorithm = strv[32];
	list->AllowSMIMEEncryptionAlgorithmNegotiation = strv[33];
	list->AllowSMIMESoftCerts = strv[34];
	list->AllowBrowser = strv[35];
	list->AllowConsumerEmail = strv[36];
	list->AllowRemoteDesktop = strv[37];
	list->AllowInternetSharing = strv[38];
	UnapprovedList = strv[39];
	ApprovedList  = strv[40];

	strx = g_strsplit (UnapprovedList, app_separator, 0);
	for(i=0; i< g_strv_length (strx); i++) {
		list->UnapprovedInROMApplicationList = g_slist_append(list->UnapprovedInROMApplicationList, strx[i]);
	}

	strx = g_strsplit (ApprovedList, app_separator, 0);
	for(i=0; i< g_strv_length (strx); i++) {
		list->ApprovedApplicationList = g_slist_append(list->ApprovedApplicationList, strx[i]);
	}

	

	/* We don't use g_strfreev() because we actually stole most of the
	   strings. So free the approved string and the unapproved but not the rest. */
	g_free (strv[39]);
	g_free (strv[40]);
	return TRUE;
}

