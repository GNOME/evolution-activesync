/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
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

#include "eas-connection-errors.h"

GQuark eas_connection_error_quark (void)
{
    static GQuark quark = 0;

    if (G_UNLIKELY (quark == 0))
    {
        const gchar *string = "eas-connection-error-quark";
        quark = g_quark_from_static_string (string);
    }

    return quark;
}

EasError common_status_error_map[] =
{
    {EAS_COMMON_STATUS_STATUSUNRECOGNIZED, "Unrecognised status"},  //100
    {EAS_COMMON_STATUS_INVALIDCONTENT, "The body of the HTTP request sent by the client is invalid"},
    {EAS_COMMON_STATUS_INVALIDWBXML, "The request contains WBXML but it could not be decoded into XML"},
    {EAS_COMMON_STATUS_INVALIDXML, "The XML provided in the request does not follow the protocol requirements"},
    {EAS_COMMON_STATUS_INVALIDDATETIME, "The request contains a timestamp that could not be parsed into a valid date and time."},
    {EAS_COMMON_STATUS_INVALIDCOMBINATIONOFIDS, "The request contains a combination of parameters that is invalid."},
    {EAS_COMMON_STATUS_INVALIDIDS, "The request contains one or more IDs that could not be parsed into valid values"},
    {EAS_COMMON_STATUS_INVALIDMIME, "The request contains MIME that could not be parsed"},
    {EAS_COMMON_STATUS_DEVICEIDMISSINGORINVALID, "The device ID is either missing or has an invalid format."},
    {EAS_COMMON_STATUS_DEVICETYPEMISSINGORINVALID, "The device type is either missing or has an invalid format"},
    {EAS_COMMON_STATUS_SERVERERROR, "The server encountered an unknown error, the device SHOULD NOT retry later"},
    {EAS_COMMON_STATUS_SERVERERRORRETRYLATER, "The server encountered an unknown error, the device SHOULD retry later"},
    {EAS_COMMON_STATUS_ACTIVEDIRECTORYACCESSDENIED, "The server does not have access to read/write to an object in the directory service"},
    {EAS_COMMON_STATUS_MAILBOXQUOTAEXCEEDED, "The mailbox has reached its size quota"},
    {EAS_COMMON_STATUS_MAILBOXSERVEROFFLINE, "The mailbox server is offline"},
    {EAS_COMMON_STATUS_SENDQUOTAEXCEEDED, "The request would exceed the send quota."},
    {EAS_COMMON_STATUS_MESSAGERECIPIENTUNRESOLVED, "One of the recipients could not be resolved to an e-mail address"},
    {EAS_COMMON_STATUS_MESSAGEREPLYNOTALLOWED, "The mailbox server will not allow a reply of this message"},
    {EAS_COMMON_STATUS_MESSAGEPREVIOUSLYSENT, "The message was already sent in a previous request"},
    {EAS_COMMON_STATUS_MESSAGEHASNORECIPIENT, "The message being sent contains no recipient."},
    {EAS_COMMON_STATUS_MAILSUBMISSIONFAILED, "The server failed to submit the message for delivery."},
    {EAS_COMMON_STATUS_MESSAGEREPLYFAILED, "The server failed to create a reply message."},
    {EAS_COMMON_STATUS_ATTACHMENTISTOOLARGE, "The attachment is too large to be processed by this request."},
    {EAS_COMMON_STATUS_USERHASNOMAILBOX, "A mailbox could not be found for the user"},
    {EAS_COMMON_STATUS_USERCANNOTBEANONYMOUS, "The request was sent without credentials. Anonymous requests are not allowed"},
    {EAS_COMMON_STATUS_USERPRINCIPALCOULDNOTBEFOUND, "The user was not found in the directory service."},
    {EAS_COMMON_STATUS_USERDISABLEDFORSYNC, "The user object in the directory service indicates that this user is not allowed to use ActiveSyn"},
    {EAS_COMMON_STATUS_USERONNEWMAILBOXCANNOTSYNC, "The server is configured to prevent users from syncing"},
    {EAS_COMMON_STATUS_USERONLEGACYMAILBOXCANNOTSYNC, "The server is configured to prevent users on legacy servers from syncing."},
    {EAS_COMMON_STATUS_DEVICEISBLOCKEDFORTHISUSER, "The user is configured to allow only some devices to sync. This device is not the allowed device."},
    {EAS_COMMON_STATUS_ACCESSDENIED, "The user is not allowed to perform that request."},
    {EAS_COMMON_STATUS_ACCOUNTDISABLED, "The user's account is disabled."},
    {EAS_COMMON_STATUS_SYNCSTATENOTFOUND, "The server’s data file that contains the state of the client was unexpectedly missing. The next request will likely answer a sync key error and the device will be forced to do full sync"},
    {EAS_COMMON_STATUS_SYNCSTATELOCKED, "The server’s data file that contains the state of the client is locked, possibly because the mailbox is being moved or was recently moved"},
    {EAS_COMMON_STATUS_SYNCSTATECORRUPT, "The server’s data file that contains the state of the client appears to be corrupt."},
    {EAS_COMMON_STATUS_SYNCSTATEALREADYEXISTS, "The server’s data file that contains the state of the client already exists. This can happen with two initial syncs are executed concurrently"},
    {EAS_COMMON_STATUS_SYNCSTATEVERSIONINVALID, "The version of the server’s data file that contains the state of the client is invalid."},
    {EAS_COMMON_STATUS_COMMANDNOTSUPPORTED, "The command is not supported by this server."},
    {EAS_COMMON_STATUS_VERSIONNOTSUPPORTED, "The command is not supported in the protocol version specified."},
    {EAS_COMMON_STATUS_DEVICENOTFULLYPROVISIONABLE, "The device uses a protocol version that cannot send all the policy settings the admin enabled."},
    {EAS_COMMON_STATUS_REMOTEWIPEREQUESTED, "A remote wipe was requested. The device SHOULD provision to get the request and then do another provision to acknowledge it"},
    {EAS_COMMON_STATUS_LEGACYDEVICEONSTRICTPOLICY, "A policy is in place but the device is not provisionable."},
    {EAS_COMMON_STATUS_DEVICENOTPROVISIONED, "There is a policy in place; the device needs to provision."},
    {EAS_COMMON_STATUS_POLICYREFRESH, "The policy is configured to be refreshed every few hours. The device needs to re-provision."},
    {EAS_COMMON_STATUS_INVALIDPOLICYKEY, "The device's policy key is invalid. The policy has probably changed on the server. The device needs to re-provision"},
    {EAS_COMMON_STATUS_EXTERNALLYMANAGEDDEVICESNOTALLOWED, "The device claimed to be externally managed, but the server doesn't allow externally managed devices to sync"},
    {EAS_COMMON_STATUS_NORECURRENCEINCALENDAR, "The request tried to forward an occurrence of a meeting that has no recurrence."},
    {EAS_COMMON_STATUS_UNEXPECTEDITEMCLASS, "The request tried to operate on a type of items unknown to the server."},
    {EAS_COMMON_STATUS_REMOTESERVERHASNOSSL, "The request needs to be proxied to another server but that server doesn't have SSL enabled. This server is configured to only proxy requests to servers with SSL enabled"},
    {EAS_COMMON_STATUS_INVALIDSTOREDREQUEST, "The server had stored the previous request from that device. The device needs to send the full request again"},
    {EAS_COMMON_STATUS_ITEMNOTFOUND, "The ItemId value specified in the SmartReply command or SmartForward command request could not be found in the mailbox"},
    {EAS_COMMON_STATUS_TOOMANYFOLDERS, "The mailbox contains too many folders. By default, the mailbox cannot contain more than 1000 folders"},
    {EAS_COMMON_STATUS_NOFOLDERSFOUND, "The mailbox contains no folders."},
    {EAS_COMMON_STATUS_ITEMSLOSTAFTERMOVE, "After moving items to the destination folder, some of those items could not be found"},
    {EAS_COMMON_STATUS_FAILUREINMOVEOPERATION, "The mailbox server returned an unknown error while moving items"},
    {EAS_COMMON_STATUS_MOVECOMMANDDISALLOWEDFORNONPERSISTENTMOVEACTION, "An ItemOperations command request to move a conversation is missing the MoveAlways element"},
    {EAS_COMMON_STATUS_MOVECOMMANDINVALIDDESTINATIONFOLDER, "The destination folder for the move is invalid"},
    {EAS_COMMON_STATUS_STATUSUNRECOGNIZED, "Unrecognised status"},  //157
    {EAS_COMMON_STATUS_STATUSUNRECOGNIZED, "Unrecognised status"},  //158
    {EAS_COMMON_STATUS_STATUSUNRECOGNIZED, "Unrecognised status"},  //159
    {EAS_COMMON_STATUS_AVAILABILITYTOOMANYRECIPIENTS, "The command has reached the maximum number of recipients that it can request availability for."},
    {EAS_COMMON_STATUS_AVAILABILITYDLLIMITREACHED, "The size of the distribution list is larger than the availability service is configured to process."},
    {EAS_COMMON_STATUS_AVAILABILITYTRANSIENTFAILURE, "Availability service request failed with a transient error."},
    {EAS_COMMON_STATUS_AVAILABILITYFAILURE, "Availability service request failed with an error."},
    {EAS_COMMON_STATUS_BODYPARTPREFERENCETYPENOTSUPPORTED, "The BodyPartPreference node has an unsupported Type element"},
    {EAS_COMMON_STATUS_DEVICEINFORMATIONREQUIRED, "The required DeviceInformation element is missing in the Provision request"},
    {EAS_COMMON_STATUS_INVALIDACCOUNTID, "The AccountId value is not valid"},
    {EAS_COMMON_STATUS_ACCOUNTSENDDISABLED, "The AccountId value specified in the request does not support sending e-mail."},
    {EAS_COMMON_STATUS_IRM_FEATUREDISABLED, "The Information Rights Management feature is disabled."},
    {EAS_COMMON_STATUS_IRM_TRANSIENTERROR, "Information Rights Management encountered an error"},
    {EAS_COMMON_STATUS_IRM_PERMANENTERROR, "Information Rights Management encountered an error.  "},
    {EAS_COMMON_STATUS_IRM_INVALIDTEMPLATEID, "The Template ID value is not valid."},
    {EAS_COMMON_STATUS_IRM_OPERATIONNOTPERMITTED, "Information Rights Management does not support the specified operation"},
    {EAS_COMMON_STATUS_NOPICTURE, "The user does not have a contact photo"},
    {EAS_COMMON_STATUS_PICTURETOOLARGE, "The contact photo exceeds the size limit set by the MaxSize element"},
    {EAS_COMMON_STATUS_PICTURELIMITREACHED, "The number of contact photos returned exceeds the size limit set by the MaxPictures element "},
    {EAS_COMMON_STATUS_BODYPART_CONVERSATIONTOOLARGE, "The conversation is too large to compute the body parts. Try requesting the body of the item again, without body parts"},
    {EAS_COMMON_STATUS_MAXIMUMDEVICESREACHED, "The user's account has too many device partnerships. Delete partnerships on the server before proceeding."},
};


EasError sync_status_error_map[] =
{
    {EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
    {0, ""},  //1, not an error case
    {EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //2
    {EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCKEY, "Sync error: Invalid synchronization key"},
    {EAS_CONNECTION_SYNC_ERROR_PROTOCOLERROR, "Sync error: Request that does not comply with the specification requirements"},
    {EAS_CONNECTION_SYNC_ERROR_SERVERERROR, "Sync error: Server misconfiguration, temporary system issue, or bad item"},
    {EAS_CONNECTION_SYNC_ERROR_CONVERSIONERROR, "Sync error: malformed or invalid item sent"},
    {EAS_CONNECTION_SYNC_ERROR_CONFLICTERROR, "Sync error: client tried to change an item for which server changes take precedence"},
    {EAS_CONNECTION_SYNC_ERROR_OBJECTNOTFOUND, "Sync error: Fetch or Change operation that has an id no longer valid on the server"},
    {EAS_CONNECTION_SYNC_ERROR_MAILBOXFULL, "Sync error: User account could be out of disk space"},
    {EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //10
    {EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  // 11
    {EAS_CONNECTION_SYNC_ERROR_FOLDERHIERARCHYCHANGED, "Sync error: Mailbox folders are not synchronized, need FolderSync first"},
    {EAS_CONNECTION_SYNC_ERROR_REQUESTINCOMPLETE, "Sync error: sync request not complete, cached set of notify-able collections missing, resend a full Sync request"},
    {EAS_CONNECTION_SYNC_ERROR_INVALIDWAITORHEARTBEAT, "Sync error: Invalid Wait or HeartbeatInterval value, update the Wait element value according to the Limit and resend the request"},
    {EAS_CONNECTION_SYNC_ERROR_INVALIDSYNCCOMMAND, "Sync error: Too many collections are included in the Sync request, sync fewer folders"},
    {EAS_CONNECTION_SYNC_ERROR_RETRY, "Sync error: Something on the server caused a retriable error."},
    {EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"}  //17
};


EasError itemoperations_status_error_map[] =
{
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
    {0, ""},  //1, success
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_PROTOCOLERROR, "Protocol error - protocol violation/XML validation error."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_SERVERERROR, "Server error"},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_BADURI, "Document library access - The specified Uniform Resource Identifier (URI) is bad."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_DOCLIBACCESSDENIED, "Document library - Access denied."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_OBJECTNOTFOUND, "Document library - The object was not found or access denied."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_FAILEDTOCONNECT, "Document library - Failed to connect to the server."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_INVALIDBYTERANGE, "The byte-range is invalid or too large"},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_UNKNOWNSTORE, "The store is unknown or unsupported."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_FILEEMPTY, "The file is empty"},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_TOOLARGE, "The requested data size is too large"},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_IOFAILURE, "Failed to download file because of input/output (I/O) failure."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"}, // 13
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_CONVERSIONFAILED, "Mailbox fetch provider - The item failed conversion."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_INVALIDATTACHMENT, "Attachment fetch provider - Attachment or attachment ID is invalid."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_RESOURCEACCESSDENIED, "Access to the resource is denied."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_PARTIALCOMPLETE, "Partial success; the command completed partially."},
    {EAS_CONNECTION_ITEMOPERATIONS_ERROR_CREDENTIALSREQUIRED, "Credentials required."},
	{EAS_CONNECTION_ITEMOPERATIONS_ERROR_STATUSUNRECOGNIZED, "Unrecognised itemoperations status"}  //19	
};


EasError ping_status_error_map[] =
{
    {EAS_CONNECTION_PING_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
    {0, ""},  //1
    {EAS_CONNECTION_PING_ERROR_FOLDERS_UPDATED, ""},
    {EAS_CONNECTION_PING_ERROR_PARAMETER, "Parameter Error - one or more parameters missing from request"},
    {EAS_CONNECTION_PING_ERROR_PROTOCOL, "Protocol error - protocol violation/XML validation error."},
    {EAS_CONNECTION_PING_ERROR_HEARTBEAT_INTERVAL, "Heartbeat Interval outside of allowed parameters - modify and retry"},
    {EAS_CONNECTION_PING_ERROR_FOLDER, "Too Many Folders Selected."},
    {EAS_CONNECTION_PING_ERROR_FOLDER_SYNC, "Folder hierarchy is wrong - resync."}
};
EasError moveitems_status_error_map[] =
{
    {EAS_CONNECTION_MOVEITEMS_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
    {EAS_CONNECTION_MOVEITEMS_ERROR_INVALID_SRC_ID, "Invalid source collection ID or invalid source Item ID. Issue a FolderSync then a Sync command for the SrcFldId and reissue the MoveItems command request if the items are still present in this folder"}, 
    {EAS_CONNECTION_MOVEITEMS_ERROR_INVALID_DST_ID, "Destination folder ID not recognised by server (may have been deleted). Issue a FolderSync then use a valid folder id"},
    {0, ""},  // 3, success
    {EAS_CONNECTION_MOVEITEMS_ERROR_SRC_AND_DST_SAME, "Source and destination folder IDs are the same"},
    {EAS_CONNECTION_MOVEITEMS_ERROR_MULTIPLE_DST, "More than one Destination folder id was included in the request or an item with that name already exists"},
    {EAS_CONNECTION_MOVEITEMS_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //6	
    {EAS_CONNECTION_MOVEITEMS_ERROR_SRC_OR_DST_LOCKED, "Source or destination item was locked. Transient server condition, retry"},
	{EAS_CONNECTION_MOVEITEMS_ERROR_STATUSUNRECOGNIZED, "Unrecognised moveitems status"}  //8
};
	

