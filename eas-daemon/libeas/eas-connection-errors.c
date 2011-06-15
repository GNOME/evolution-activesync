/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 * eas-daemon is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * eas-daemon is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-connection-errors.h"

GQuark eas_connection_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0)) {
		const gchar *string = "eas-connection-error-quark";
		quark = g_quark_from_static_string (string);
	}

	return quark;
}

// TODO think of a more elegant way to do the status->error mappings?

EasError common_status_error_map[] = {
	{EAS_CONNECTION_ERROR_STATUSUNRECOGNIZED, "Unrecognised status"},  //100	
	{EAS_CONNECTION_ERROR_INVALIDCONTENT, "The body of the HTTP request sent by the client is invalid"},  
	{EAS_CONNECTION_ERROR_INVALIDWBXML, "The request contains WBXML but it could not be decoded into XML"},
	{EAS_CONNECTION_ERROR_INVALIDXML, "The XML provided in the request does not follow the protocol requirements"},
	{EAS_CONNECTION_ERROR_INVALIDDATETIME, "The request contains a timestamp that could not be parsed into a valid date and time."},
	{EAS_CONNECTION_ERROR_INVALIDCOMBINATIONOFIDS, "The request contains a combination of parameters that is invalid."},
	{EAS_CONNECTION_ERROR_INVALIDIDS, "The request contains one or more IDs that could not be parsed into valid values"},
	{EAS_CONNECTION_ERROR_INVALIDMIME, "The request contains MIME that could not be parsed"},
	{EAS_CONNECTION_ERROR_DEVICEIDMISSINGORINVALID, "The device ID is either missing or has an invalid format."},
	{EAS_CONNECTION_ERROR_DEVICETYPEMISSINGORINVALID, "The device type is either missing or has an invalid format"},
	{EAS_CONNECTION_ERROR_SERVERERROR, "The server encountered an unknown error, the device SHOULD NOT retry later"},
	{EAS_CONNECTION_ERROR_SERVERERRORRETRYLATER, "The server encountered an unknown error, the device SHOULD retry later"},
	{EAS_CONNECTION_ERROR_ACTIVEDIRECTORYACCESSDENIED, "The server does not have access to read/write to an object in the directory service"},
	{EAS_CONNECTION_ERROR_MAILBOXQUOTAEXCEEDED, "The mailbox has reached its size quota"},
	{EAS_CONNECTION_ERROR_MAILBOXSERVEROFFLINE, "The mailbox server is offline"},
	{EAS_CONNECTION_ERROR_SENDQUOTAEXCEEDED, "The request would exceed the send quota."},
	{EAS_CONNECTION_ERROR_MESSAGERECIPIENTUNRESOLVED, "One of the recipients could not be resolved to an e-mail address"},
	{EAS_CONNECTION_ERROR_MESSAGEREPLYNOTALLOWED, "The mailbox server will not allow a reply of this message"},
	{EAS_CONNECTION_ERROR_MESSAGEPREVIOUSLYSENT, "The message was already sent in a previous request"},
	{EAS_CONNECTION_ERROR_MESSAGEHASNORECIPIENT, "The message being sent contains no recipient."},
	{EAS_CONNECTION_ERROR_MAILSUBMISSIONFAILED, "The server failed to submit the message for delivery."},
	{EAS_CONNECTION_ERROR_MESSAGEREPLYFAILED, "The server failed to create a reply message."},
	{EAS_CONNECTION_ERROR_ATTACHMENTISTOOLARGE, "The attachment is too large to be processed by this request."},
	{EAS_CONNECTION_ERROR_USERHASNOMAILBOX, "A mailbox could not be found for the user"},
	{EAS_CONNECTION_ERROR_USERCANNOTBEANONYMOUS, "The request was sent without credentials. Anonymous requests are not allowed"},
	{EAS_CONNECTION_ERROR_USERPRINCIPALCOULDNOTBEFOUND, "The user was not found in the directory service."},
	{EAS_CONNECTION_ERROR_USERDISABLEDFORSYNC, "The user object in the directory service indicates that this user is not allowed to use ActiveSyn"},
	{EAS_CONNECTION_ERROR_USERONNEWMAILBOXCANNOTSYNC, "The server is configured to prevent users from syncing"},
	{EAS_CONNECTION_ERROR_USERONLEGACYMAILBOXCANNOTSYNC, "The server is configured to prevent users on legacy servers from syncing."},
	{EAS_CONNECTION_ERROR_DEVICEISBLOCKEDFORTHISUSER, "The user is configured to allow only some devices to sync. This device is not the allowed device."},
	{EAS_CONNECTION_ERROR_ACCESSDENIED, "The user is not allowed to perform that request."},
	{EAS_CONNECTION_ERROR_ACCOUNTDISABLED, "The user's account is disabled."},
	{EAS_CONNECTION_ERROR_SYNCSTATENOTFOUND, "The server’s data file that contains the state of the client was unexpectedly missing. The next request will likely answer a sync key error and the device will be forced to do full sync"},
	{EAS_CONNECTION_ERROR_SYNCSTATELOCKED, "The server’s data file that contains the state of the client is locked, possibly because the mailbox is being moved or was recently moved"},
	{EAS_CONNECTION_ERROR_SYNCSTATECORRUPT, "The server’s data file that contains the state of the client appears to be corrupt."},
	{EAS_CONNECTION_ERROR_SYNCSTATEALREADYEXISTS, "The server’s data file that contains the state of the client already exists. This can happen with two initial syncs are executed concurrently"},
	{EAS_CONNECTION_ERROR_SYNCSTATEVERSIONINVALID, "The version of the server’s data file that contains the state of the client is invalid."},
	{EAS_CONNECTION_ERROR_COMMANDNOTSUPPORTED, "The command is not supported by this server."},
	{EAS_CONNECTION_ERROR_VERSIONNOTSUPPORTED, "The command is not supported in the protocol version specified."},
	{EAS_CONNECTION_ERROR_DEVICENOTFULLYPROVISIONABLE, "The device uses a protocol version that cannot send all the policy settings the admin enabled."},
	{EAS_CONNECTION_ERROR_REMOTEWIPEREQUESTED, "A remote wipe was requested. The device SHOULD provision to get the request and then do another provision to acknowledge it"},
	{EAS_CONNECTION_ERROR_LEGACYDEVICEONSTRICTPOLICY, "A policy is in place but the device is not provisionable."},
	{EAS_CONNECTION_ERROR_DEVICENOTPROVISIONED, "There is a policy in place; the device needs to provision."},
	{EAS_CONNECTION_ERROR_POLICYREFRESH, "The policy is configured to be refreshed every few hours. The device needs to re-provision."},
	{EAS_CONNECTION_ERROR_INVALIDPOLICYKEY, "The device's policy key is invalid. The policy has probably changed on the server. The device needs to re-provision"},
	{EAS_CONNECTION_ERROR_EXTERNALLYMANAGEDDEVICESNOTALLOWED, "The device claimed to be externally managed, but the server doesn't allow externally managed devices to sync"},
	{EAS_CONNECTION_ERROR_NORECURRENCEINCALENDAR, "The request tried to forward an occurrence of a meeting that has no recurrence."},
	{EAS_CONNECTION_ERROR_UNEXPECTEDITEMCLASS, "The request tried to operate on a type of items unknown to the server."},
	{EAS_CONNECTION_ERROR_REMOTESERVERHASNOSSL, "The request needs to be proxied to another server but that server doesn't have SSL enabled. This server is configured to only proxy requests to servers with SSL enabled"},
	{EAS_CONNECTION_ERROR_INVALIDSTOREDREQUEST, "The server had stored the previous request from that device. The device needs to send the full request again"},
	{EAS_CONNECTION_ERROR_ITEMNOTFOUND, "The ItemId value specified in the SmartReply command or SmartForward command request could not be found in the mailbox"},
	{EAS_CONNECTION_ERROR_TOOMANYFOLDERS, "The mailbox contains too many folders. By default, the mailbox cannot contain more than 1000 folders"},
	{EAS_CONNECTION_ERROR_NOFOLDERSFOUND, "The mailbox contains no folders."},
	{EAS_CONNECTION_ERROR_ITEMSLOSTAFTERMOVE, "After moving items to the destination folder, some of those items could not be found"},
	{EAS_CONNECTION_ERROR_FAILUREINMOVEOPERATION, "The mailbox server returned an unknown error while moving items"},
	{EAS_CONNECTION_ERROR_MOVECOMMANDDISALLOWEDFORNONPERSISTENTMOVEACTION, "An ItemOperations command request to move a conversation is missing the MoveAlways element"},
	{EAS_CONNECTION_ERROR_MOVECOMMANDINVALIDDESTINATIONFOLDER, "The destination folder for the move is invalid"},  
	{EAS_CONNECTION_ERROR_STATUSUNRECOGNIZED, "Unrecognised status"},  //157
	{EAS_CONNECTION_ERROR_STATUSUNRECOGNIZED, "Unrecognised status"},  //158
	{EAS_CONNECTION_ERROR_STATUSUNRECOGNIZED, "Unrecognised status"},  //159
	{EAS_CONNECTION_ERROR_AVAILABILITYTOOMANYRECIPIENTS, "The command has reached the maximum number of recipients that it can request availability for."},
	{EAS_CONNECTION_ERROR_AVAILABILITYDLLIMITREACHED, "The size of the distribution list is larger than the availability service is configured to process."},
	{EAS_CONNECTION_ERROR_AVAILABILITYTRANSIENTFAILURE, "Availability service request failed with a transient error."},
	{EAS_CONNECTION_ERROR_AVAILABILITYFAILURE, "Availability service request failed with an error."},
	{EAS_CONNECTION_ERROR_BODYPARTPREFERENCETYPENOTSUPPORTED, "The BodyPartPreference node has an unsupported Type element"},
	{EAS_CONNECTION_ERROR_DEVICEINFORMATIONREQUIRED, "The required DeviceInformation element is missing in the Provision request"},
	{EAS_CONNECTION_ERROR_INVALIDACCOUNTID, "The AccountId value is not valid"},
	{EAS_CONNECTION_ERROR_ACCOUNTSENDDISABLED, "The AccountId value specified in the request does not support sending e-mail."},
	{EAS_CONNECTION_ERROR_IRM_FEATUREDISABLED, "The Information Rights Management feature is disabled."},
	{EAS_CONNECTION_ERROR_IRM_TRANSIENTERROR, "Information Rights Management encountered an error"},
	{EAS_CONNECTION_ERROR_IRM_PERMANENTERROR, "Information Rights Management encountered an error.  "},
	{EAS_CONNECTION_ERROR_IRM_INVALIDTEMPLATEID, "The Template ID value is not valid."},
	{EAS_CONNECTION_ERROR_IRM_OPERATIONNOTPERMITTED, "Information Rights Management does not support the specified operation"},
	{EAS_CONNECTION_ERROR_NOPICTURE, "The user does not have a contact photo"},
	{EAS_CONNECTION_ERROR_PICTURETOOLARGE, "The contact photo exceeds the size limit set by the MaxSize element"},
	{EAS_CONNECTION_ERROR_PICTURELIMITREACHED, "The number of contact photos returned exceeds the size limit set by the MaxPictures element "},
	{EAS_CONNECTION_ERROR_BODYPART_CONVERSATIONTOOLARGE, "The conversation is too large to compute the body parts. Try requesting the body of the item again, without body parts"},
	{EAS_CONNECTION_ERROR_MAXIMUMDEVICESREACHED, "The user's account has too many device partnerships. Delete partnerships on the server before proceeding."},
};


EasError sync_status_error_map[] = {
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
	{EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"}	//17
};


EasError itemoperations_status_error_map[] = {
	{EAS_CONNECTION_ITEMOPERATIONS_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
	{0, ""},  //1
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
	{EAS_CONNECTION_ITEMOPERATIONS_ERROR_PARTIALCOMPLETE,"Partial success; the command completed partially."},
	{EAS_CONNECTION_ITEMOPERATIONS_ERROR_CREDENTIALSREQUIRED, "Credentials required."}
};

