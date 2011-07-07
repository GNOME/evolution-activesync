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

#include "eas-sync-req.h"
#include "eas-sync-msg.h"
#include "eas-sync-folder-msg.h"
#include "eas-connection-errors.h"
#include "serialise_utils.h"
#include <string.h>
#include "../../libeasaccount/src/eas-account-list.h"

G_DEFINE_TYPE (EasSyncReq, eas_sync_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SYNC_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_REQ, EasSyncReqPrivate))

typedef enum
{
    EasSyncReqStep1 = 0,
    EasSyncReqStep2,
	EasSyncReqStep3
} EasSyncReqState;


struct _EasSyncReqPrivate
{
    EasSyncMsg* syncMsg;
    gchar* sync_key;
    EasSyncReqState state;
    gchar* accountID;
    gchar* folderID;
    EasItemType ItemType;
	EasConnection* cnc;
	EasAccount* acc;
};



static void
eas_sync_req_init (EasSyncReq *object)
{
    EasSyncReqPrivate *priv;

    object->priv = priv = EAS_SYNC_REQ_PRIVATE (object);

    g_debug ("eas_sync_req_init++");
    priv->syncMsg = NULL;
    priv->state = EasSyncReqStep1;
    priv->accountID = NULL;
    priv->folderID = NULL;
	priv->cnc = NULL;
	priv->acc = NULL;
	priv->ItemType = EAS_ITEM_NOT_SPECIFIED;
    eas_request_base_SetRequestType (&object->parent_instance,
                                     EAS_REQ_SYNC);

    g_debug ("eas_sync_req_init--");
}

static void
eas_sync_req_dispose (GObject *object)
{
    EasSyncReq *req = (EasSyncReq*) object;
    EasSyncReqPrivate *priv = req->priv;

    g_debug ("eas_sync_req_dispose++");

    if (priv->syncMsg)
    {
        g_object_unref (priv->syncMsg);
		priv->syncMsg = NULL;
    }
	if (priv->acc)
    {
        g_object_unref (priv->acc);
		priv->acc = NULL;
    }

	G_OBJECT_CLASS (eas_sync_req_parent_class)->dispose (object);

    g_debug ("eas_sync_req_dispose--");

}

static void
eas_sync_req_finalize (GObject *object)
{
    EasSyncReq *req = (EasSyncReq*) object;
    EasSyncReqPrivate *priv = req->priv;

    g_debug ("eas_sync_req_finalize++");

    g_free (priv->accountID);
    g_free (priv->folderID);

    G_OBJECT_CLASS (eas_sync_req_parent_class)->finalize (object);

    g_debug ("eas_sync_req_finalize--");

}

static void
eas_sync_req_class_init (EasSyncReqClass *klass)
{
    GObjectClass* object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (EasSyncReqPrivate));

    object_class->dispose = eas_sync_req_dispose;
    object_class->finalize = eas_sync_req_finalize;
}

EasSyncReq *eas_sync_req_new (const gchar* syncKey, 
                              const gchar* accountID, 
                              const gchar* folderId, 
                              EasItemType type,
                              DBusGMethodInvocation *context) 
{
    EasSyncReq* self = g_object_new (EAS_TYPE_SYNC_REQ, NULL);
    EasSyncReqPrivate *priv = self->priv;
	EasAccountList *account_list = NULL;
	GConfClient* client = NULL;

	client = gconf_client_get_default();
	g_assert(client != NULL);
	/* Get list of accounts from gconf repository */
	account_list = eas_account_list_new (client);
	g_assert(account_list != NULL);

	priv->acc = eas_account_list_find(account_list, EAS_ACCOUNT_FIND_ACCOUNT_UID, accountID);

    g_debug ("eas_sync_req_new++");

	g_assert (syncKey);
	g_assert (accountID);
	
    priv->sync_key = g_strdup (syncKey);
    priv->accountID = g_strdup (accountID);

	// If we have a folderId which isn't NULL or an empty string
	if (folderId && g_strcmp0("", folderId))
	{
		priv->folderID = g_strdup (folderId);
	}
    priv->ItemType = type;

	priv->cnc = eas_request_base_GetConnection (&self->parent_instance);

    eas_request_base_SetContext (&self->parent_instance, context);

    g_debug ("eas_sync_req_new--");
    return self;
}

gboolean
eas_sync_req_Activate (EasSyncReq *self,
                       GError** error)
{
    gboolean ret = FALSE;
    EasSyncReqPrivate* priv;
    xmlDoc *doc;
    gboolean getChanges = FALSE;

    g_debug ("eas_sync_req_activate++");

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    priv = self->priv;
	g_debug("about to check folderID");

	// if Folder Id is not set, then for contacts, we get a default,
	// if the default has not been set yet, then call sync folder hierarchy,
	// which will write them to gconf 
	if(priv->folderID == NULL)
	{
		g_debug("folder id missing - look for default");
		priv->state = EasSyncReqStep1;
		switch(priv->ItemType)
		{
			case EAS_ITEM_CALENDAR:
			{
				g_debug("get calendar folder");
				priv->folderID = g_strdup(eas_account_get_calendar_folder(priv->acc));
				if(priv->folderID != NULL)
				{
					g_debug("default folder id for calendar = [%s]", priv->folderID);
					priv->state = EasSyncReqStep2;
				}
			}
			break;
			case EAS_ITEM_CONTACT:
			{
				g_debug("default folder id for contacts = [%s]", priv->folderID);
				priv->folderID = g_strdup(eas_account_get_contact_folder(priv->acc));
				if(priv->folderID != NULL)
				{
					priv->state = EasSyncReqStep2;
				}
			}
			break;
		}
	}
	else
	{
		priv->state = EasSyncReqStep2;
	}

	g_debug("default missing");

	if(priv->state == EasSyncReqStep1)
	{
		g_debug("default folder id missing - do folder sync");
		//we don't have a default folder ID so we must do a folder sync first
		priv->syncMsg = eas_sync_folder_msg_new ("0", priv->accountID);
		if (!priv->syncMsg)
		{
		    ret = FALSE;
		    // set the error
		    g_set_error (error, EAS_CONNECTION_ERROR,
		                 EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
		                 ("out of memory"));
		    goto finish;
		}
		//build request msg
		doc = eas_sync_folder_msg_build_message (priv->syncMsg);
		if (!doc)
		{
		    ret = FALSE;
		
			// set the error
		    g_set_error (error, EAS_CONNECTION_ERROR,
		                 EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
		                 ("out of memory"));
		    goto finish;
		}

		g_debug ("eas_sync_req_activate - send message");
		ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
		                                   "FolderSync",
		                                   doc,
		                                   (struct _EasRequestBase *) self,
		                                   error);
		
	}
	else 
	{

		g_debug ("eas_sync_req_activate - new Sync  mesg");

		//create sync  msg type
		priv->syncMsg = eas_sync_msg_new (priv->sync_key, priv->accountID, priv->folderID, priv->ItemType);
		if (!priv->syncMsg)
		{
		    ret = FALSE;
		    // set the error
		    g_set_error (error, EAS_CONNECTION_ERROR,
		                 EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
		                 ("out of memory"));
		    goto finish;
		}

		g_debug ("eas_sync_req_activate- syncKey = %s", priv->sync_key);

		//if syncKey is not 0, then we are not doing a first time sync and only need to send one message
		// so we  move state machine forward
		if (g_strcmp0 (priv->sync_key, "0"))
		{
		    g_debug ("switching state");
		    priv->state = EasSyncReqStep3;
		    getChanges = TRUE;
		}

		g_debug ("eas_sync_req_activate - build messsage");
		//build request msg
		doc = eas_sync_msg_build_message (priv->syncMsg, getChanges, NULL, NULL, NULL);
		if (!doc)
		{
		    ret = FALSE;
		
			// set the error
		    g_set_error (error, EAS_CONNECTION_ERROR,
		                 EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
		                 ("out of memory"));
		    goto finish;
		}

		g_debug ("eas_sync_req_activate - send message");
		ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
		                                   "Sync",
		                                   doc,
		                                   (struct _EasRequestBase *) self,
		                                   error);
	}
finish:
    if (!ret)
    {
        g_assert (error == NULL || *error != NULL);

        if (priv->syncMsg)
		{
			g_object_unref (priv->syncMsg);
			priv->syncMsg = NULL;
		}
    }
	
    g_debug ("eas_sync_req_activate--");

    return ret;
}

gboolean
eas_sync_req_MessageComplete (EasSyncReq *self, xmlDoc* doc, GError* error_in)
{
    gboolean cleanup = FALSE;
    gboolean ret = TRUE;
    GError *error = NULL;
    EasSyncReqPrivate* priv = self->priv;
	gchar *ret_sync_key = NULL;
    gboolean ret_more_available = FALSE;
    gchar** ret_added_item_array = NULL;
    gchar** ret_deleted_item_array = NULL;
    gchar** ret_changed_item_array = NULL;

	GSList* added_items = NULL;
    GSList* updated_items = NULL;
    GSList* deleted_items = NULL;

    g_debug ("eas_sync_req_MessageComplete++");

    // if an error occurred, store it and signal daemon
    if (error_in)
    {
		xmlFreeDoc (doc);
		ret = FALSE;
        error = error_in;
        goto finish;
    }


    switch (priv->state)
    {
        default:
        {
			ret = FALSE;
			g_set_error (&error, EAS_CONNECTION_ERROR,
				         EAS_CONNECTION_SYNC_ERROR_INVALIDSTATE,
				         ("Invalid state"));
        }
        break;
		case EasSyncReqStep1:
		{
			// We do some direct assigns lower down so this should always be true.
			g_assert(!priv->folderID);
			
			ret = eas_sync_folder_msg_parse_response ((EasSyncFolderMsg*)priv->syncMsg, doc, &error);

			xmlFreeDoc (doc);
			if (!ret)
			{
				g_assert (error != NULL);
				goto finish;
			}
			priv->state = EasSyncReqStep2;
			if(priv->ItemType == EAS_ITEM_CALENDAR)
			{
				// cannot get from gconf - as the update takes too long - get from sync msg response
				priv->folderID = g_strdup(eas_sync_folder_msg_get_def_cal_folder((EasSyncFolderMsg*)priv->syncMsg));
			}else
			{
				// cannot get from gconf - as the update takes too long - get from sync msg response
				priv->folderID = g_strdup(eas_sync_folder_msg_get_def_con_folder((EasSyncFolderMsg*)priv->syncMsg));
			}
			 //clean up old message
            if (priv->syncMsg)
            {
                g_object_unref (priv->syncMsg);
				priv->syncMsg = NULL;
            }
			//create sync  msg type
			priv->syncMsg = eas_sync_msg_new (priv->sync_key, priv->accountID, priv->folderID, priv->ItemType);
			if (!priv->syncMsg)
			{
				ret = FALSE;
				// set the error
				g_set_error (error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
				goto finish;
			}

			g_debug ("eas_sync_req_activate- syncKey = %s", priv->sync_key);

			g_debug ("eas_sync_req_activate - build messsage");
			//build request msg
			doc = eas_sync_msg_build_message (priv->syncMsg, FALSE, NULL, NULL, NULL);
			if (!doc)
			{
				ret = FALSE;
		
				// set the error
				g_set_error (error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
				goto finish;
			}

			g_debug ("eas_sync_req_MEssageCompete - send  sync message");
			ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
				                               "Sync",
				                               doc,
				                               (struct _EasRequestBase *) self,
				                               error);
			
		}
		break;

        //We have started a first time sync, and need to get the sync Key from the result, and then do the proper sync
        case EasSyncReqStep2:
        {
			gchar * syncKey = NULL;
			
			ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
			xmlFreeDoc (doc);
			if (!ret)
			{
				g_assert (error != NULL);
				goto finish;
			}
            //get syncKey
            syncKey = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));

            g_debug ("eas_sync_req synckey = %s", syncKey);

            //clean up old message
            if (priv->syncMsg)
            {
                g_object_unref (priv->syncMsg);
                priv->syncMsg = NULL;
            }

            //create new message with new syncKey
            priv->syncMsg = eas_sync_msg_new (syncKey, priv->accountID, priv->folderID, priv->ItemType);
			g_free(syncKey);
            //build request msg
            doc = eas_sync_msg_build_message (priv->syncMsg, TRUE, NULL, NULL, NULL);
			if (!doc)
			{
				g_set_error (&error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
				ret = FALSE;
				goto finish;
			}
            //move to new state
            priv->state = EasSyncReqStep3;

            ret = eas_connection_send_request (eas_request_base_GetConnection (&self->parent_instance),
                                               "Sync",
                                               doc, (struct _EasRequestBase *) self,
                                               &error);
            if (!ret)
            {
                g_assert (error != NULL);
                goto finish;
            }

        }
        break;

        //we did a proper sync, so we need to inform the daemon that we have finished, so that it can continue and get the data
        case EasSyncReqStep3:
        {
		    ret = eas_sync_msg_parse_response (priv->syncMsg, doc, &error);
			xmlFreeDoc (doc);
			if (!ret)
			{
				g_assert (error != NULL);
				goto finish;
			}
			//finished state machine - req needs to be cleanup up by connection object
			cleanup = TRUE;
            g_debug ("eas_sync_req_MessageComplete step 3");
			ret_more_available = eas_sync_msg_get_more_available(priv->syncMsg);
			ret_sync_key  = g_strdup (eas_sync_msg_get_syncKey (priv->syncMsg));
			added_items   = eas_sync_msg_get_added_items (priv->syncMsg);
			updated_items = eas_sync_msg_get_updated_items (priv->syncMsg);
			deleted_items = eas_sync_msg_get_deleted_items (priv->syncMsg);

			switch(priv->ItemType)
			{
				case EAS_ITEM_MAIL:
				{
					ret = build_serialised_email_info_array (&ret_added_item_array, added_items, &error);
					if (ret)
					{
						ret = build_serialised_email_info_array (&ret_changed_item_array, updated_items, &error);
						if (ret)
						{
							ret = build_serialised_email_info_array (&ret_deleted_item_array, deleted_items, &error);
						}
					}
					 if (!ret)
					{
						g_set_error (&error, EAS_CONNECTION_ERROR,
				             EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
				             ("out of memory"));
						goto finish;
					}
				}
				break;
				case EAS_ITEM_CALENDAR:
				case EAS_ITEM_CONTACT:
				{
					if (build_serialised_calendar_info_array (&ret_added_item_array, added_items, &error))
					{
						if (build_serialised_calendar_info_array (&ret_changed_item_array, updated_items, &error))
						{
							build_serialised_calendar_info_array (&ret_deleted_item_array, deleted_items, &error);
						}
					}
				}
				break;
				default:
				{
				ret = FALSE;
				g_set_error (&error, EAS_CONNECTION_ERROR,
						     EAS_CONNECTION_SYNC_ERROR_INVALIDTYPE,
						     ("Invalid type"));
				goto finish;
				}
			}
			dbus_g_method_return (eas_request_base_GetContext (&self->parent_instance),
                              ret_sync_key,
                              ret_more_available,
                              ret_added_item_array,
                              ret_deleted_item_array,
                              ret_changed_item_array);

        }
        break;
    }

finish:
	if (!ret)
    {
        g_debug ("returning error %s", error->message);
        g_assert (error != NULL);
        dbus_g_method_return_error (eas_request_base_GetContext (&self->parent_instance), error);
        g_error_free (error);
		error = NULL;
		
		cleanup = TRUE;
    }
	g_free(ret_sync_key);
	g_strfreev(ret_added_item_array);
	g_strfreev(ret_deleted_item_array);
	g_strfreev(ret_changed_item_array);
    g_debug ("eas_sync_req_MessageComplete--");

	return cleanup;
}
