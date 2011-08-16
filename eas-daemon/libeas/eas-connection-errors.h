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

#ifndef EAS_ERRORS_H
#define EAS_ERRORS_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

GQuark eas_connection_error_quark (void);

#define EAS_CONNECTION_ERROR (eas_connection_error_quark ())

#define EAS_TYPE_CONNECTION_ERROR (eas_connection_error_get_type ())
GType eas_connection_error_get_type (void);

// note: the error values in this enum do not match anything in MSFT docs, the values are our own:
enum _EasConnectionError {
	EAS_CONNECTION_ERROR_FAILED, // 0
	EAS_CONNECTION_ERROR_FILEERROR,
	EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
	EAS_CONNECTION_ERROR_BADARG,
	EAS_CONNECTION_ERROR_WBXMLERROR,
	EAS_CONNECTION_ERROR_SOUPERROR,
	EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	// generic error for use by response parsers
	EAS_CONNECTION_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_ERROR_ACCOUNTNOTFOUND,
	EAS_CONNECTION_ERROR_BADREQUESTSTATE,
	EAS_CONNECTION_ERROR_XMLTOOLARGETODOM,
	EAS_CONNECTION_ERROR_NOTSUPPORTED,
	EAS_CONNECTION_ERROR_CANCELLED,

	/*  Provision status errors  */
	EAS_CONNECTION_PROVISION_ERROR_PROTOCOLERROR,
	EAS_CONNECTION_PROVISION_ERROR_GENERALSERVERERROR,
	EAS_CONNECTION_PROVISION_ERROR_DEVICE_EXTERNALLY_MANAGED,
	EAS_CONNECTION_PROVISION_ERROR_NOCLIENTPOLICYEXISTS,
	EAS_CONNECTION_PROVISION_ERROR_UNKNOWNPOLICYTYPE,
	EAS_CONNECTION_PROVISION_ERROR_CORRUPTSERVERPOLICYDATA,
	EAS_CONNECTION_PROVISION_ERROR_ACKINGWRONGPOLICYKEY,
	EAS_CONNECTION_PROVISION_ERROR_STATUSUNRECOGNIZED,

	/* Sync status errors  */
	EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCKEY,
	EAS_CONNECTION_SYNC_ERROR_PROTOCOLERROR,
	EAS_CONNECTION_SYNC_ERROR_SERVERERROR,
	EAS_CONNECTION_SYNC_ERROR_CONVERSIONERROR,
	EAS_CONNECTION_SYNC_ERROR_CONFLICTERROR,
	EAS_CONNECTION_SYNC_ERROR_OBJECTNOTFOUND,
	EAS_CONNECTION_SYNC_ERROR_MAILBOXFULL,
	EAS_CONNECTION_SYNC_ERROR_FOLDERHIERARCHYCHANGED,
	EAS_CONNECTION_SYNC_ERROR_REQUESTINCOMPLETE,
	EAS_CONNECTION_SYNC_ERROR_INVALIDWAITORHEARTBEAT,
	EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCCOMMAND,
	EAS_CONNECTION_SYNC_ERROR_RETRY,
	EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_SYNC_ERROR_INVALIDSTATE,
	EAS_CONNECTION_SYNC_ERROR_INVALIDTYPE,

	/* FolderSync status errors */
	EAS_CONNECTION_FOLDER_SYNC_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_SERVERERROR,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_TIMEOUT,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_INVALIDSYNCKEY,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_BADLYFORMATTEDREQUEST,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_UNKNOWNERROR,
	EAS_CONNECTION_FOLDER_SYNC_ERROR_CODEUNKNOWN,

	/* ItemOperations status errors */
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_PROTOCOLERROR,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_SERVERERROR,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_BADURI,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_DOCLIBACCESSDENIED,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_OBJECTNOTFOUND,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_FAILEDTOCONNECT,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_INVALIDBYTERANGE,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_UNKNOWNSTORE,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_FILEEMPTY,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_TOOLARGE,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_IOFAILURE,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_CONVERSIONFAILED,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_INVALIDATTACHMENT,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_RESOURCEACCESSDENIED,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_PARTIALCOMPLETE,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_CREDENTIALSREQUIRED,
	EAS_CONNECTION_ITEMOPERATIONS_ERROR_STATUSUNRECOGNIZED,

	/* Ping status errors */
	EAS_CONNECTION_PING_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_PING_ERROR_FOLDERS_UPDATED,
	EAS_CONNECTION_PING_ERROR_PARAMETER,
	EAS_CONNECTION_PING_ERROR_PROTOCOL,
	EAS_CONNECTION_PING_ERROR_HEARTBEAT_INTERVAL,
	EAS_CONNECTION_PING_ERROR_FOLDER,
	EAS_CONNECTION_PING_ERROR_FOLDER_SYNC,

	/* MoveItems status errors */
	EAS_CONNECTION_MOVEITEMS_ERROR_INVALID_SRC_ID,
	EAS_CONNECTION_MOVEITEMS_ERROR_INVALID_DST_ID,
	EAS_CONNECTION_MOVEITEMS_ERROR_SRC_AND_DST_SAME,
	EAS_CONNECTION_MOVEITEMS_ERROR_MULTIPLE_DST,
	EAS_CONNECTION_MOVEITEMS_ERROR_SRC_OR_DST_LOCKED,
	EAS_CONNECTION_MOVEITEMS_ERROR_STATUSUNRECOGNIZED,

	/* GetItemEstimate status errors */
	EAS_CONNECTION_GETITEMESTIMATE_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_GETITEMESTIMATE_ERROR_INVALID_COLLECTION,
	EAS_CONNECTION_GETITEMESTIMATE_ERROR_BAD_SYNC_STATE,
	EAS_CONNECTION_GETITEMESTIMATE_ERROR_INVALID_SYNC_KEY,

	/* Common status errors */
	EAS_CONNECTION_COMMON_ERROR_STATUSUNRECOGNIZED,
	EAS_CONNECTION_COMMON_ERROR_INVALIDCONTENT,
	EAS_CONNECTION_COMMON_ERROR_INVALIDWBXML,
	EAS_CONNECTION_COMMON_ERROR_INVALIDXML,
	EAS_CONNECTION_COMMON_ERROR_INVALIDDATETIME,
	EAS_CONNECTION_COMMON_ERROR_INVALIDCOMBINATIONOFIDS,
	EAS_CONNECTION_COMMON_ERROR_INVALIDIDS,
	EAS_CONNECTION_COMMON_ERROR_INVALIDMIME,
	EAS_CONNECTION_COMMON_ERROR_DEVICEIDMISSINGORINVALID,
	EAS_CONNECTION_COMMON_ERROR_DEVICETYPEMISSINGORINVALID,
	EAS_CONNECTION_COMMON_ERROR_SERVERERROR,
	EAS_CONNECTION_COMMON_ERROR_SERVERERRORRETRYLATER,
	EAS_CONNECTION_COMMON_ERROR_ACTIVEDIRECTORYACCESSDENIED,
	EAS_CONNECTION_COMMON_ERROR_MAILBOXQUOTAEXCEEDED,
	EAS_CONNECTION_COMMON_ERROR_MAILBOXSERVEROFFLINE,
	EAS_CONNECTION_COMMON_ERROR_SENDQUOTAEXCEEDED,
	EAS_CONNECTION_COMMON_ERROR_MESSAGERECIPIENTUNRESOLVED,
	EAS_CONNECTION_COMMON_ERROR_MESSAGEREPLYNOTALLOWED,
	EAS_CONNECTION_COMMON_ERROR_MESSAGEPREVIOUSLYSENT,
	EAS_CONNECTION_COMMON_ERROR_MESSAGEHASNORECIPIENT,
	EAS_CONNECTION_COMMON_ERROR_MAILSUBMISSIONFAILED,
	EAS_CONNECTION_COMMON_ERROR_MESSAGEREPLYFAILED,
	EAS_CONNECTION_COMMON_ERROR_ATTACHMENTISTOOLARGE,
	EAS_CONNECTION_COMMON_ERROR_USERHASNOMAILBOX,
	EAS_CONNECTION_COMMON_ERROR_USERCANNOTBEANONYMOUS,
	EAS_CONNECTION_COMMON_ERROR_USERPRINCIPALCOULDNOTBEFOUND,
	EAS_CONNECTION_COMMON_ERROR_USERDISABLEDFORSYNC,
	EAS_CONNECTION_COMMON_ERROR_USERONNEWMAILBOXCANNOTSYNC,
	EAS_CONNECTION_COMMON_ERROR_USERONLEGACYMAILBOXCANNOTSYNC,
	EAS_CONNECTION_COMMON_ERROR_DEVICEISBLOCKEDFORTHISUSER,
	EAS_CONNECTION_COMMON_ERROR_ACCESSDENIED,
	EAS_CONNECTION_COMMON_ERROR_ACCOUNTDISABLED,
	EAS_CONNECTION_COMMON_ERROR_SYNCSTATENOTFOUND,
	EAS_CONNECTION_COMMON_ERROR_SYNCSTATELOCKED,
	EAS_CONNECTION_COMMON_ERROR_SYNCSTATECORRUPT,
	EAS_CONNECTION_COMMON_ERROR_SYNCSTATEALREADYEXISTS,
	EAS_CONNECTION_COMMON_ERROR_SYNCSTATEVERSIONINVALID,
	EAS_CONNECTION_COMMON_ERROR_COMMANDNOTSUPPORTED,
	EAS_CONNECTION_COMMON_ERROR_VERSIONNOTSUPPORTED,
	EAS_CONNECTION_COMMON_ERROR_DEVICENOTFULLYPROVISIONABLE,
	EAS_CONNECTION_COMMON_ERROR_REMOTEWIPEREQUESTED,
	EAS_CONNECTION_COMMON_ERROR_LEGACYDEVICEONSTRICTPOLICY,
	EAS_CONNECTION_COMMON_ERROR_DEVICENOTPROVISIONED,
	EAS_CONNECTION_COMMON_ERROR_POLICYREFRESH,
	EAS_CONNECTION_COMMON_ERROR_INVALIDPOLICYKEY,
	EAS_CONNECTION_COMMON_ERROR_EXTERNALLYMANAGEDDEVICESNOTALLOWED,
	EAS_CONNECTION_COMMON_ERROR_NORECURRENCEINCALENDAR,
	EAS_CONNECTION_COMMON_ERROR_UNEXPECTEDITEMCLASS,
	EAS_CONNECTION_COMMON_ERROR_REMOTESERVERHASNOSSL,
	EAS_CONNECTION_COMMON_ERROR_INVALIDSTOREDREQUEST,
	EAS_CONNECTION_COMMON_ERROR_ITEMNOTFOUND,
	EAS_CONNECTION_COMMON_ERROR_TOOMANYFOLDERS,
	EAS_CONNECTION_COMMON_ERROR_NOFOLDERSFOUND,
	EAS_CONNECTION_COMMON_ERROR_ITEMSLOSTAFTERMOVE,
	EAS_CONNECTION_COMMON_ERROR_FAILUREINMOVEOPERATION,
	EAS_CONNECTION_COMMON_ERROR_MOVECOMMANDDISALLOWEDFORNONPERSISTENTMOVEACTION,
	EAS_CONNECTION_COMMON_ERROR_MOVECOMMANDINVALIDDESTINATIONFOLDER,
	EAS_CONNECTION_COMMON_ERROR_AVAILABILITYTOOMANYRECIPIENTS,
	EAS_CONNECTION_COMMON_ERROR_AVAILABILITYDLLIMITREACHED,
	EAS_CONNECTION_COMMON_ERROR_AVAILABILITYTRANSIENTFAILURE,
	EAS_CONNECTION_COMMON_ERROR_AVAILABILITYFAILURE,
	EAS_CONNECTION_COMMON_ERROR_BODYPARTPREFERENCETYPENOTSUPPORTED,
	EAS_CONNECTION_COMMON_ERROR_DEVICEINFORMATIONREQUIRED,
	EAS_CONNECTION_COMMON_ERROR_INVALIDACCOUNTID,
	EAS_CONNECTION_COMMON_ERROR_ACCOUNTSENDDISABLED,
	EAS_CONNECTION_COMMON_ERROR_IRM_FEATUREDISABLED,
	EAS_CONNECTION_COMMON_ERROR_IRM_TRANSIENTERROR,
	EAS_CONNECTION_COMMON_ERROR_IRM_PERMANENTERROR,
	EAS_CONNECTION_COMMON_ERROR_IRM_INVALIDTEMPLATEID,
	EAS_CONNECTION_COMMON_ERROR_IRM_OPERATIONNOTPERMITTED,
	EAS_CONNECTION_COMMON_ERROR_NOPICTURE,
	EAS_CONNECTION_COMMON_ERROR_PICTURETOOLARGE,
	EAS_CONNECTION_COMMON_ERROR_PICTURELIMITREACHED,
	EAS_CONNECTION_COMMON_ERROR_BODYPART_CONVERSATIONTOOLARGE,
	EAS_CONNECTION_COMMON_ERROR_MAXIMUMDEVICESREACHED,
	EAS_CONNECTION_COMMON_ERROR_EXCEEDSSTATUSLIMIT,

	/* */

	EAS_CONNECTION_ERROR_LAST
} ;

typedef enum _EasConnectionError EasConnectionError;

/* EAS Status codes (see MSFT EAS Docs) */
enum _EasCommonStatus {
	EAS_COMMON_STATUS_OK = 1,
	EAS_COMMON_STATUS_STATUSUNRECOGNIZED = 100,
	EAS_COMMON_STATUS_INVALIDCONTENT = 101,
	EAS_COMMON_STATUS_INVALIDWBXML = 102,
	EAS_COMMON_STATUS_INVALIDXML = 103,
	EAS_COMMON_STATUS_INVALIDDATETIME = 104,
	EAS_COMMON_STATUS_INVALIDCOMBINATIONOFIDS = 105,
	EAS_COMMON_STATUS_INVALIDIDS = 106,
	EAS_COMMON_STATUS_INVALIDMIME = 107,
	EAS_COMMON_STATUS_DEVICEIDMISSINGORINVALID = 108,
	EAS_COMMON_STATUS_DEVICETYPEMISSINGORINVALID = 109,
	EAS_COMMON_STATUS_SERVERERROR = 110,
	EAS_COMMON_STATUS_SERVERERRORRETRYLATER = 111,
	EAS_COMMON_STATUS_ACTIVEDIRECTORYACCESSDENIED = 112,
	EAS_COMMON_STATUS_MAILBOXQUOTAEXCEEDED = 113,
	EAS_COMMON_STATUS_MAILBOXSERVEROFFLINE = 114,
	EAS_COMMON_STATUS_SENDQUOTAEXCEEDED = 115,
	EAS_COMMON_STATUS_MESSAGERECIPIENTUNRESOLVED = 116,
	EAS_COMMON_STATUS_MESSAGEREPLYNOTALLOWED = 117,
	EAS_COMMON_STATUS_MESSAGEPREVIOUSLYSENT = 118,
	EAS_COMMON_STATUS_MESSAGEHASNORECIPIENT = 119,
	EAS_COMMON_STATUS_MAILSUBMISSIONFAILED = 120,
	EAS_COMMON_STATUS_MESSAGEREPLYFAILED = 121,
	EAS_COMMON_STATUS_ATTACHMENTISTOOLARGE = 122,
	EAS_COMMON_STATUS_USERHASNOMAILBOX = 123,
	EAS_COMMON_STATUS_USERCANNOTBEANONYMOUS = 124,
	EAS_COMMON_STATUS_USERPRINCIPALCOULDNOTBEFOUND = 125,
	EAS_COMMON_STATUS_USERDISABLEDFORSYNC = 126,
	EAS_COMMON_STATUS_USERONNEWMAILBOXCANNOTSYNC = 127,
	EAS_COMMON_STATUS_USERONLEGACYMAILBOXCANNOTSYNC = 128,
	EAS_COMMON_STATUS_DEVICEISBLOCKEDFORTHISUSER = 129,
	EAS_COMMON_STATUS_ACCESSDENIED = 130,
	EAS_COMMON_STATUS_ACCOUNTDISABLED = 131,
	EAS_COMMON_STATUS_SYNCSTATENOTFOUND = 132,
	EAS_COMMON_STATUS_SYNCSTATELOCKED = 133,
	EAS_COMMON_STATUS_SYNCSTATECORRUPT = 134,
	EAS_COMMON_STATUS_SYNCSTATEALREADYEXISTS = 135,
	EAS_COMMON_STATUS_SYNCSTATEVERSIONINVALID = 136,
	EAS_COMMON_STATUS_COMMANDNOTSUPPORTED = 137,
	EAS_COMMON_STATUS_VERSIONNOTSUPPORTED = 138,
	EAS_COMMON_STATUS_DEVICENOTFULLYPROVISIONABLE = 139,
	EAS_COMMON_STATUS_REMOTEWIPEREQUESTED = 140,
	EAS_COMMON_STATUS_LEGACYDEVICEONSTRICTPOLICY = 141,
	EAS_COMMON_STATUS_DEVICENOTPROVISIONED = 142,
	EAS_COMMON_STATUS_POLICYREFRESH = 143,
	EAS_COMMON_STATUS_INVALIDPOLICYKEY = 144,
	EAS_COMMON_STATUS_EXTERNALLYMANAGEDDEVICESNOTALLOWED = 145,
	EAS_COMMON_STATUS_NORECURRENCEINCALENDAR = 146,
	EAS_COMMON_STATUS_UNEXPECTEDITEMCLASS = 147,
	EAS_COMMON_STATUS_REMOTESERVERHASNOSSL = 148,
	EAS_COMMON_STATUS_INVALIDSTOREDREQUEST = 149,
	EAS_COMMON_STATUS_ITEMNOTFOUND = 150,
	EAS_COMMON_STATUS_TOOMANYFOLDERS = 151,
	EAS_COMMON_STATUS_NOFOLDERSFOUND = 152,
	EAS_COMMON_STATUS_ITEMSLOSTAFTERMOVE = 153,
	EAS_COMMON_STATUS_FAILUREINMOVEOPERATION = 154,
	EAS_COMMON_STATUS_MOVECOMMANDDISALLOWEDFORNONPERSISTENTMOVEACTION = 155,
	EAS_COMMON_STATUS_MOVECOMMANDINVALIDDESTINATIONFOLDER = 156,
	EAS_COMMON_STATUS_AVAILABILITYTOOMANYRECIPIENTS = 160,
	EAS_COMMON_STATUS_AVAILABILITYDLLIMITREACHED = 161,
	EAS_COMMON_STATUS_AVAILABILITYTRANSIENTFAILURE = 162,
	EAS_COMMON_STATUS_AVAILABILITYFAILURE = 163,
	EAS_COMMON_STATUS_BODYPARTPREFERENCETYPENOTSUPPORTED = 164,
	EAS_COMMON_STATUS_DEVICEINFORMATIONREQUIRED = 165,
	EAS_COMMON_STATUS_INVALIDACCOUNTID = 166,
	EAS_COMMON_STATUS_ACCOUNTSENDDISABLED = 167,
	EAS_COMMON_STATUS_IRM_FEATUREDISABLED = 168,
	EAS_COMMON_STATUS_IRM_TRANSIENTERROR = 169,
	EAS_COMMON_STATUS_IRM_PERMANENTERROR = 170,
	EAS_COMMON_STATUS_IRM_INVALIDTEMPLATEID = 171,
	EAS_COMMON_STATUS_IRM_OPERATIONNOTPERMITTED = 172,
	EAS_COMMON_STATUS_NOPICTURE = 173,
	EAS_COMMON_STATUS_PICTURETOOLARGE = 174,
	EAS_COMMON_STATUS_PICTURELIMITREACHED = 175,
	EAS_COMMON_STATUS_BODYPART_CONVERSATIONTOOLARGE = 176,
	EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED = 177,
	EAS_COMMON_STATUS_EXCEEDSSTATUSLIMIT	// no common status above 177 currently
};


// All enums from this point are for internal use only and will not be returned on the APIs:

// note: the following status enums are mainly for documentation (they show the status codes defined by msft)
// only the last item in each enum is used elsewhere in code:

enum _EasItemOperationsStatus {
	EAS_ITEMOPERATIONS_STATUS_PROTOCOLERROR = 2,
	EAS_ITEMOPERATIONS_STATUS_SERVERERROR = 3,
	EAS_ITEMOPERATIONS_STATUS_BADURI = 4,
	EAS_ITEMOPERATIONS_STATUS_DOCLIBACCESSDENIED = 5,
	EAS_ITEMOPERATIONS_STATUS_OBJECTNOTFOUND = 6,
	EAS_ITEMOPERATIONS_STATUS_FAILEDTOCONNECT = 7,
	EAS_ITEMOPERATIONS_STATUS_INVALIDEBYTERANGE = 8,
	EAS_ITEMOPERATIONS_STATUS_UNKNOWNSTORE = 9,
	EAS_ITEMOPERATIONS_STATUS_FILEEMPTY = 10,
	EAS_ITEMOPERATIONS_STATUS_TOOLARGE = 11,
	EAS_ITEMOPERATIONS_STATUS_IOFAILURE = 12,
	EAS_ITEMOPERATIONS_STATUS_CONVERSIONFAILED = 14,
	EAS_ITEMOPERATIONS_STATUS_INVALIDATTACHMENT = 15,
	EAS_ITEMOPERATIONS_STATUS_RESOURCEACCESSDENIED = 16,
	EAS_ITEMOPERATIONS_STATUS_PARTIALCOMPLETE = 17,
	EAS_ITEMOPERATIONS_STATUS_CREDENTIALSREQUIRED = 18,
	EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT,   // no itemoperations status spec'd above 18 currently
};

enum _EasSyncStatus {
	EAS_SYNC_STATUS_INVALIDSYNCKEY = 3,
	EAS_SYNC_STATUS_PROTOCOLERROR = 4,
	EAS_SYNC_STATUS_SERVERERROR = 5,
	EAS_SYNC_STATUS_CONVERSIONERROR = 6,
	EAS_SYNC_STATUS_CONFLICTERROR = 7,
	EAS_SYNC_STATUS_OBJECTNOTFOUND = 8,
	EAS_SYNC_STATUS_MAILBOXFULL = 9,
	EAS_SYNC_STATUS_FOLDERHIERARCHYCHANGED = 12,
	EAS_SYNC_STATUS_REQUESTINCOMPLETE = 13,
	EAS_SYNC_STATUS_INVALIDWAITORHEARTBEAT = 14,
	EAS_SYNC_STATUS_INVALIDSYNCCOMMAND = 15,
	EAS_SYNC_STATUS_RETRY = 16,
	EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT,   // no sync status spec'd above 16 currently
};

enum _EasFolderSyncStatus {
	EAS_FOLDER_SYNC_STATUS_SERVERERROR = 6,
	EAS_FOLDER_SYNC_STATUS_TIMEOUT = 8,
	EAS_FOLDER_SYNC_STATUS_INVALIDSYNCKEY = 9,
	EAS_FOLDER_SYNC_STATUS_BADLYFORMATTEDREQUEST = 10,
	EAS_FOLDER_SYNC_STATUS_UNKNOWNERROR = 11,
	EAS_FOLDER_SYNC_STATUS_CODEUNKNOWN = 12,
	EAS_FOLDER_SYNC_STATUS_EXCEEDSSTATUSLIMIT = 13,   // no sync status spec'd above 12 currently
};


enum _EasPingStatus {
	EAS_PING_STATUS_FOLDERS_UPDATED = 2,
	EAS_PING_STATUS_PARAMETER_ERROR = 3,
	EAS_PING_STATUS_PROTOCOL_ERROR = 4,
	EAS_PING_STATUS_HEARTBEAT_INTERVAL_ERROR = 5,
	EAS_PING_STATUS_FOLDER_ERROR = 6,
	EAS_PING_STATUS_FOLDER_HIERARCHY_ERROR = 7,

	EAS_PING_STATUS_EXCEEDSSTATUSLIMIT,   // no sync status spec'd above 7 currently
};

enum _EasMoveItemsStatus {
	EAS_MOVEITEMS_STATUS_INVALID_SRC_ID = 1,		// note 1 != success
	EAS_MOVEITEMS_STATUS_INVALID_DST_ID = 2,
	EAS_MOVEITEMS_STATUS_SUCCESS = 3,
	EAS_MOVEITEMS_STATUS_SRC_AND_DST_SAME = 4,
	EAS_MOVEITEMS_STATUS_MULTIPLE_DST = 5,
	EAS_MOVEITEMS_STATUS_SRC_OR_DST_LOCKED = 7,

	EAS_MOVEITEMS_STATUS_EXCEEDSSTATUSLIMIT,   // no sync status spec'd above 7 currently
};

enum _EasGetItemEstimateStatus {
	// 1 == success
	EAS_GETITEMESTIMATE_STATUS_INVALID_COLLECTION = 2,
	EAS_GETITEMESTIMATE_STATUS_BAD_SYNC_STATE = 3,
	EAS_GETITEMESTIMATE_STATUS_INVALID_SYNC_KEY = 4,

	EAS_GETITEMESTIMATE_STATUS_EXCEEDSSTATUSLIMIT,   // no sync status spec'd above 4 currently
};

struct _EasError {
	EasConnectionError code;
	const gchar *message;
};

typedef struct _EasError EasError;

extern EasError sync_status_error_map[];
extern EasError itemoperations_status_error_map[];
extern EasError moveitems_status_error_map[];
extern EasError common_status_error_map[];
extern EasError ping_status_error_map[];
extern EasError folder_sync_status_error_map[];
extern EasError get_item_estimate_status_error_map[];

G_END_DECLS

#endif
