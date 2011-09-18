/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 *
 */

#ifndef _EAS_PROVISION_LIST_H_
#define _EAS_PROVISION_LIST_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_PROVISION_LIST             (eas_provision_list_get_type ())
#define EAS_PROVISION_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_PROVISION_LIST, EasProvisionList))
#define EAS_PROVISION_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_PROVISION_LIST, EasProvisionListClass))
#define EAS_IS_PROVISION_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_PROVISION_LIST))
#define EAS_IS_PROVISION_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_PROVISION_LIST))
#define EAS_PROVISION_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_PROVISION_LIST, EasProvisionListClass))

typedef struct _EasProvisionListClass EasProvisionListClass;
typedef struct _EasProvisionList EasProvisionList;

struct _EasProvisionListClass {
	GObjectClass parent_class;
};

struct _EasProvisionList {
	GObject parent_instance;

	gchar* DevicePasswordEnabled;
	gchar* AlphaNumericDevicePasswordRequired;
	gchar* PasswordRecoveryEnabled;
	gchar* RequireStorageCardEncryption;
	gchar* AttachmentsEnabled;
	gchar* MinDevicePasswordLength;
	gchar* MaxInactivityTimeDeviceLock;
	gchar* MaxDevicePasswordFailedAttempts;
	gchar* MaxAttachmentSize;
	gchar* AllowSimpleDevicePassword;
	gchar* DevicePasswordExpiration;
	gchar* DevicePasswordHistory;
	gchar* AllowStorageCard;
	gchar* AllowCamera;
	gchar* RequireDeviceEncryption;
	gchar* AllowUnsignedApplications;
	gchar* AllowUnsignedInstallationPackages;
	gchar* MinDevicePasswordComplexCharacters;
	gchar* AllowWifi;
	gchar* AllowTextMessaging;
	gchar* AllowPOPIMAPEmail;
	gchar* AllowBluetooth;
	gchar* AllowIrDA;
	gchar* RequireManualSyncWhenRoaming;
	gchar* AllowDesktopSync;
	gchar* MaxCalendarAgeFilter;
	gchar* AllowHTMLEmail;
	gchar* MaxEmailAgeFilter;
	gchar* MaxEmailBodyTruncationSize;
	gchar* MaxEmailHTMLBodyTruncationSize;
	gchar* RequireSignedSMIMEMessages;
	gchar* RequireEncryptedSMIMEMessages;
	gchar* RequireSignedSMIMEAlgorithm;
	gchar* RequireEncryptionSMIMEAlgorithm;
	gchar* AllowSMIMEEncryptionAlgorithmNegotiation;
	gchar* AllowSMIMESoftCerts;
	gchar* AllowBrowser;
	gchar* AllowConsumerEmail;
	gchar* AllowRemoteDesktop;
	gchar* AllowInternetSharing;
	GSList* UnapprovedInROMApplicationList;
	GSList* ApprovedApplicationList;
};

GType eas_provision_list_get_type (void) G_GNUC_CONST;

EasProvisionList* eas_provision_list_new();

gboolean
eas_provision_list_serialise (EasProvisionList* list, gchar **result);

gboolean
eas_provision_list_deserialise (EasProvisionList* folder, const gchar *data);

G_END_DECLS

#endif /* _EAS_PROVISION_LIST_H_ */
