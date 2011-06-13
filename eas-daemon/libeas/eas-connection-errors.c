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

EasError status_error_map[] = {
	{EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //0
	{EAS_CONNECTION_SYNC_ERROR_STATUSUNRECOGNIZED, "Unrecognised sync status"},  //1
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

