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

#ifndef EAS_CONNECTION_ERRORS_H
#define EAS_CONNECTION_ERRORS_H

#include <glib.h>
#include <glib-object.h>

#include <eas-errors.h>

G_BEGIN_DECLS

#define EAS_TYPE_CONNECTION_ERROR (eas_connection_error_get_type ())
GType eas_connection_error_get_type (void);

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

#endif /* EAS_CONNECTION_ERRORS_H */
