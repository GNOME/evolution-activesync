/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-connection-errors.h"
#include "eas-sync-msg.h"
#include "eas-email-info-translator.h"
#include "eas-cal-info-translator.h"

struct _EasSyncMsgPrivate
{
	GSList* added_items;
	GSList* updated_items;
	GSList* deleted_items;

	gboolean more_available;
	gchar* sync_key;
	gchar* folderID;
	gint   account_id;
	
	EasItemType ItemType;
};

#define EAS_SYNC_MSG_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SYNC_MSG, EasSyncMsgPrivate))


G_DEFINE_TYPE (EasSyncMsg, eas_sync_msg, EAS_TYPE_MSG_BASE);

//static void eas_sync_parse_item_add(EasSyncMsg *self, xmlNode *node, GError** error);

static void
eas_sync_msg_init (EasSyncMsg *object)
{
	EasSyncMsgPrivate *priv;
	g_debug("eas_sync_msg_init++");

	object->priv = priv = EAS_SYNC_MSG_PRIVATE(object);
	
	priv->more_available = FALSE;
	priv->sync_key = NULL;
	priv->folderID = NULL;
	priv->account_id = -1;
	g_debug("eas_sync_msg_init--");
}

static void
eas_sync_msg_finalize (GObject *object)
{
	EasSyncMsg *msg = (EasSyncMsg *)object;
	EasSyncMsgPrivate *priv = msg->priv;

	g_free(priv->sync_key);
	g_free(priv->folderID);
	
	G_OBJECT_CLASS (eas_sync_msg_parent_class)->finalize (object);
}

static void
eas_sync_msg_class_init (EasSyncMsgClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasMsgBaseClass* parent_class = EAS_MSG_BASE_CLASS (klass);
	void *tmp = parent_class;
	tmp = object_class;
	
	g_type_class_add_private (klass, sizeof (EasSyncMsgPrivate));

	object_class->finalize = eas_sync_msg_finalize;
}

EasSyncMsg*
eas_sync_msg_new (const gchar* syncKey, const gint accountId, const gchar *folderID, const EasItemType type)
{
	EasSyncMsg* msg = NULL;
	EasSyncMsgPrivate *priv = NULL;

	msg = g_object_new (EAS_TYPE_SYNC_MSG, NULL);
	priv = msg->priv;

	priv->more_available = FALSE;
	priv->sync_key = g_strdup(syncKey);
	priv->account_id = accountId;
	priv->folderID = g_strdup(folderID);
	priv->ItemType = type;

	return msg;
}

xmlDoc*
eas_sync_msg_build_message (EasSyncMsg* self, gboolean getChanges, GSList *added, GSList *updated, GSList *deleted)
{
	EasSyncMsgPrivate *priv = self->priv;
    xmlDoc  *doc   = NULL;
    xmlNode *node  = NULL, 
	        *child = NULL,
	        *collection = NULL,
	        *options = NULL,
	        *body_pref = NULL;
    xmlNs   *ns    = NULL;

    doc = xmlNewDoc ( (xmlChar *) "1.0");
    node = xmlNewDocNode (doc, NULL, (xmlChar*)"Sync", NULL);
    xmlDocSetRootElement (doc, node);
    
    xmlCreateIntSubset(doc, 
                       (xmlChar*)"ActiveSync", 
                       (xmlChar*)"-//MICROSOFT//DTD ActiveSync//EN", 
                       (xmlChar*)"http://www.microsoft.com/");

	ns = xmlNewNs (node, (xmlChar *)"AirSync:",NULL);
	xmlNewNs (node, (xmlChar *)"AirSyncBase:", (xmlChar *)"airsyncbase");
	child = xmlNewChild(node, NULL, (xmlChar *)"Collections", NULL);
	collection = xmlNewChild(child, NULL, (xmlChar *)"Collection", NULL);
	xmlNewChild(collection, NULL, (xmlChar *)"SyncKey", (xmlChar*)priv->sync_key);
	xmlNewChild(collection, NULL, (xmlChar *)"CollectionId", (xmlChar*)priv->folderID);
	//if get changes = true - means we are pulling from the server
	if(getChanges){
		xmlNewChild(collection, NULL, (xmlChar *)"DeletesAsMoves", (xmlChar*)"1");
		xmlNewChild(collection, NULL, (xmlChar *)"GetChanges", (xmlChar*)"1");
		xmlNewChild(collection, NULL, (xmlChar *)"WindowSize", (xmlChar*)"100");
   
		if(priv->ItemType == EAS_ITEM_MAIL){
        
        options = xmlNewChild(collection, NULL, (xmlChar *)"Options", NULL);
            xmlNewChild(options, NULL, (xmlChar *)"FilterType", (xmlChar*)"0");
            xmlNewChild(options, NULL, (xmlChar *)"MIMESupport", (xmlChar*)"2");
            xmlNewChild(options, NULL, (xmlChar *)"MIMETruncation", (xmlChar*)"1"); // First 4KiB

            body_pref = xmlNewChild(options, NULL, (xmlChar *)"airsyncbase:BodyPreference", NULL);
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:Type", (xmlChar*)"4"); // Plain text 1, HTML 2, MIME 4
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:TruncationSize", (xmlChar*)"200000");
		}
		else if(priv->ItemType == EAS_ITEM_CALENDAR){
        
        options = xmlNewChild(collection, NULL, (xmlChar *)"Options", NULL);
            xmlNewChild(options, NULL, (xmlChar *)"FilterType", (xmlChar*)"0");

            body_pref = xmlNewChild(options, NULL, (xmlChar *)"airsyncbase:BodyPreference", NULL);
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:Type", (xmlChar*)"1"); // Plain text 1, HTML 2, MIME 4
            xmlNewChild(body_pref, NULL, (xmlChar *)"airsyncbase:TruncationSize", (xmlChar*)"200000");
		}
    }
    //get changes = false, we are pushing changes to the server. Check the lists of items, and build correct message.
    else{
		GSList * iterator;
		xmlNewChild(collection, NULL, (xmlChar *)"DeletesAsMoves", (xmlChar*)"1");
		xmlNewChild(collection, NULL, (xmlChar *)"GetChanges", (xmlChar*)"0");
		//if any of the lists are not null we need to add commands element
		if(added || updated || deleted)
		{
			xmlNode *command = xmlNewChild(collection, NULL, (xmlChar *)"Commands", NULL);
			if(added){
				for (iterator = added; iterator; iterator = iterator->next) {
					//choose translator based on data type
					switch(priv->ItemType)
					{
						default:
						{
							g_debug ("Unknown Data Type  %d", priv->ItemType);
						}
						break;
						case EAS_ITEM_MAIL:
						{
							g_error("Trying to do Add with Mail type - This is not allowed");
						}
						case EAS_ITEM_CALENDAR:
						{
							xmlNode *added = xmlNewChild(command, NULL, (xmlChar *)"Add", NULL);
							xmlNewNs (node, (xmlChar *)"Calendar:", (xmlChar *)"calendar");	
							if(iterator->data){
								//TODO: call translator to get client ID and  encoded application data
								//gchar *serialised_calendar = (gchar *)iterator->data;	
								xmlNode *app_data = NULL;
								EasCalInfo *cal_info =(EasCalInfo*) iterator->data;	
								
								// create the server_id node
								xmlNewChild(added, NULL, (xmlChar *)"ClientId", (xmlChar*)cal_info->client_id);
								app_data = xmlNewChild(added, NULL, (xmlChar *)"ApplicationData", NULL);
								// translator deals with app data
								eas_cal_info_translator_parse_request(doc, app_data, cal_info);
								// TODO error handling and freeing
							}
						}
						break;
						
					}		
				
				}
			}
			if(updated){
				for (iterator = updated; iterator; iterator = iterator->next) {
					xmlNode *update = xmlNewChild(command, NULL, (xmlChar *)"Change", NULL);
					//choose translator based on data type					
					switch(priv->ItemType)
					{
						default:
						{
							g_debug ("Unknown Data Type  %d", priv->ItemType);
						}
						break;
						case EAS_ITEM_MAIL:
						{
							gchar *serialised_email = (gchar *)updated->data;
							EasEmailInfo *email_info = eas_email_info_new ();
							
							xmlNewNs (node, (xmlChar *)"Email:", (xmlChar *)"email");

							if(eas_email_info_deserialise(email_info, serialised_email))
							{
							xmlNode *app_data = NULL;
							// create the server_id node
							xmlNewChild(update, NULL, (xmlChar *)"ServerId", (xmlChar*)email_info->server_id);
							app_data = xmlNewChild(update, NULL, (xmlChar *)"ApplicationData", NULL);
							// call translator to get encoded application data
							eas_email_info_translator_build_update_request(doc, app_data, email_info);
							g_object_unref(email_info);
							}
							// TODO error handling
						}
						break;
						case EAS_ITEM_CALENDAR:
						{
							xmlNewNs (node, (xmlChar *)"Calendar:", (xmlChar *)"calendar");	
							if(iterator->data){
								//TODO: call translator to get client ID and  encoded application data
								//gchar *serialised_calendar = (gchar *)iterator->data;	
											
								EasCalInfo *cal_info =(EasCalInfo*) iterator->data;	
								xmlNode *app_data = NULL;
								// create the server_id node
								xmlNewChild(update, NULL, (xmlChar *)"ServerId", (xmlChar*)cal_info->server_id);
								app_data = xmlNewChild(update, NULL, (xmlChar *)"ApplicationData", NULL);
								// translator deals with app data
								eas_cal_info_translator_parse_request(doc, app_data, cal_info);
								// TODO error handling and freeing
							}
						}			
					}	
				}
			}
			if(deleted){
				for (iterator = deleted; iterator; iterator = iterator->next) {
					xmlNode *delete = xmlNewChild(command, NULL, (xmlChar *)"Delete", NULL);
					xmlNewChild(delete, NULL, (xmlChar *)"ServerId", iterator->data);
				}

			}
		}
    }    

    return doc;
}


gboolean
eas_sync_msg_parse_response (EasSyncMsg* self, xmlDoc *doc, GError** error)
{
	EasError error_details;
	gboolean ret = TRUE;
	EasSyncMsgPrivate *priv = self->priv;
	xmlNode *node = NULL,
					*appData = NULL;
					
	gchar *item_server_id = NULL;
	gchar *item_client_id = NULL;

	g_debug ("eas_sync_msg_parse_response ++");
	
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	
    if (!doc) {
        g_warning ("folder_sync response XML is empty");
		// Note not setting error here as empty doc is valid	
		goto finish;
    }
    node = xmlDocGetRootElement(doc);
    
    //TODO: parse response correctly
    
    if (g_strcmp0((char *)node->name, "Sync")) {
		g_set_error (error, EAS_CONNECTION_ERROR,
		EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	   
		("Failed to find <Sync> element"));
        ret = FALSE;
		goto finish;		
    }
    for (node = node->children; node; node = node->next) 
	{
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Status")) 
        {
            gchar *sync_status = (gchar *)xmlNodeGetContent(node);
			EasSyncStatus sync_status_num = atoi(sync_status);			
			xmlFree(sync_status);
			if(sync_status_num != EAS_COMMON_STATUS_OK)  // not success
			{
				// TODO could also be a common status code!?
				if(sync_status_num > EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT)
				{
					sync_status_num = EAS_SYNC_STATUS_EXCEEDSSTATUSLIMIT;
				}
				error_details = sync_status_error_map[sync_status_num];
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				ret = FALSE;
				goto finish;
			}
            continue;
        }    
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Collections")) 
        {
               g_debug ("Collections:");
               break;
        }

    }
    if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
		EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	   
		("Failed to find <Collections> element"));
        ret = FALSE;
		goto finish;
    }
    
    for (node = node->children; node; node = node->next) {
    
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Collection")) 
        {
               g_debug ("Collection:");
               break;
        }

    }
    if (!node) {
		g_set_error (error, EAS_CONNECTION_ERROR,
		EAS_CONNECTION_ERROR_XMLELEMENTNOTFOUND,	   
		("Failed to find <Collection> element"));
        ret = FALSE;
		goto finish;
    }
	
    for (node = node->children; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Status")) 
        {
            gchar *sync_status = (gchar *)xmlNodeGetContent(node);
			EasSyncStatus status_num = atoi(sync_status);			
			xmlFree(sync_status);
			if(status_num != EAS_COMMON_STATUS_OK)  // not success
			{
				ret = FALSE;

				// TODO - think of a nicer way to do this?
				if((EAS_CONNECTION_ERROR_INVALIDCONTENT <= status_num) && (status_num <= EAS_CONNECTION_ERROR_MAXIMUMDEVICESREACHED))// it's a common status code
				{
					error_details = common_status_error_map[status_num - 100];
				}
				else 
				{
					if(status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT)// not pretty, but make sure we don't overrun array if new status added
						status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;

					error_details = itemoperations_status_error_map[status_num];	
				}
				g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
				goto finish;
			}
            continue;
        }		
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "SyncKey"))
		{
			priv->sync_key = (gchar *)xmlNodeGetContent(node);
			g_debug ("Got SyncKey = %s", priv->sync_key);
			continue;
		}
        if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "MoreAvailable"))
		{
			priv->more_available = TRUE;
			g_debug ("Got <MoreAvailable/>");
			continue;
		}
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Responses")) 
        {
               g_debug ("Responses:\n");
               break;
        }		
		if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Commands")) 
        {
               g_debug ("Commands:\n");
               break;
        }		
		
	}

    if (!node) {
        g_warning ("Found no <Responses> element or <Commands> element>");
		// Note not setting error here as this is valid	
		goto finish;		
    }

	if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Commands")){
		 for (node = node->children; node; node = node->next) {
	
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Add")) {
				appData = node;
			
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ServerId")) {
						item_server_id = (gchar *)xmlNodeGetContent(appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ApplicationData")) {
						gchar *flatItem = NULL;
						g_debug ("Found AppliicationData - about to parse and flatten");
						//choose translator based on data type
						switch(priv->ItemType)
						{
							default:
							{
								g_debug ("Unknown Data Type  %d", priv->ItemType);
							}
							break;
							case EAS_ITEM_MAIL:
							{

								flatItem = eas_add_email_appdata_parse_response(appData, item_server_id); 
							}
							break;
							case EAS_ITEM_CALENDAR:
							{
								flatItem = eas_cal_info_translator_parse_response(appData, item_server_id);
							}
							break;

						
						}		

						g_debug ("FlatItem = %s", flatItem);
						if(flatItem){
							g_debug ("appending to added_items");
							priv->added_items = g_slist_append(priv->added_items, flatItem);
						}
						
						continue;
					}
				}
				continue;
			}
		
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Delete")) {
				appData = node;
				// TODO Parse deleted folders
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ServerId")) {
						item_server_id = (gchar *)xmlNodeGetContent(appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						priv->deleted_items = g_slist_append(priv->deleted_items, item_server_id);
						continue;
					}
				}
				continue;
			}
		
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Change")) {
				// TODO Parse updated folders
				appData = node;
			
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ServerId")) {
						item_server_id = (gchar *)xmlNodeGetContent(appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ApplicationData")) {
						gchar *flatItem = NULL;
						g_debug ("Found AppliicationData - about to parse and flatten");
						//choose translator based on data type
						switch(priv->ItemType)
						{
							default:
							{
								g_debug ("Unknown Data Type  %d", priv->ItemType);
							}
							break;
							case EAS_ITEM_MAIL:
							{
								g_debug("calling email appdata translator for update");
								flatItem = eas_update_email_appdata_parse_response(appData, item_server_id); 
							}
							break;
							case EAS_ITEM_CALENDAR:
							{
								flatItem = eas_cal_info_translator_parse_response(appData, item_server_id);
							}
							break;
						
						}		
					
						g_debug ("FlatItem = %s", flatItem);
						if(flatItem){
							g_debug ("appending to updated_items");
							priv->updated_items = g_slist_append(priv->updated_items, flatItem);
						}
						
						continue;
					}
				}
				continue;
			}
		}
	}
	else if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Responses")){
		for (node = node->children; node; node = node->next) {
	
			if (node->type == XML_ELEMENT_NODE && !g_strcmp0((char *)node->name, "Add")) {
				gchar *flatItem = NULL;
				EasCalInfo *info = NULL;
				appData = node;
				for (appData = appData->children; appData; appData = appData->next) {
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ClientId")) {
						item_client_id = (gchar *)xmlNodeGetContent(appData);
						g_debug ("Found clientID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "ServerId")) {
						item_server_id = (gchar *)xmlNodeGetContent(appData);
						g_debug ("Found serverID for Item = %s", item_server_id);
						continue;
					}
					if (appData->type == XML_ELEMENT_NODE && !g_strcmp0((char *)appData->name, "Status")) 
					{
						gchar *status = (gchar *)xmlNodeGetContent(appData);
						EasSyncStatus status_num = atoi(status);			
						xmlFree(status);
						if(status_num != EAS_COMMON_STATUS_OK)  // not success
						{
							ret = FALSE;

							// TODO - think of a nicer way to do this?
							if((EAS_CONNECTION_ERROR_INVALIDCONTENT <= status_num) && (status_num <= EAS_CONNECTION_ERROR_MAXIMUMDEVICESREACHED))// it's a common status code
							{
								error_details = common_status_error_map[status_num - 100];
							}
							else 
							{
								if(status_num > EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT)// not pretty, but make sure we don't overrun array if new status added
									status_num = EAS_ITEMOPERATIONS_STATUS_EXCEEDSSTATUSLIMIT;

								error_details = itemoperations_status_error_map[status_num];	
							}
							g_set_error (error, EAS_CONNECTION_ERROR, error_details.code, "%s", error_details.message);
							goto finish;
						}
						continue;
					}
				}
				info = eas_cal_info_new ();
				info->client_id = item_client_id;
				info->server_id = item_server_id;
				eas_cal_info_serialise (info, &flatItem);
				g_object_unref (info);
				item_client_id = NULL;
				item_server_id = NULL;
				if(flatItem){
					g_debug ("appending to added_items");
					priv->added_items = g_slist_append(priv->added_items, flatItem);
				}
				continue;
			}
		}
	}
	
	g_debug ("eas_sync_msg_parse_response--");

finish:
	if(!ret)
	{
		g_assert (error == NULL || *error != NULL);
	}	
	return ret;
}


GSList*
eas_sync_msg_get_added_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	g_debug("eas added items list size = %d", g_slist_length(priv->added_items));
	return priv->added_items;
}

GSList*
eas_sync_msg_get_updated_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->updated_items;
}

GSList*
eas_sync_msg_get_deleted_items (EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
	return priv->deleted_items;
}

gchar* 
eas_sync_msg_get_syncKey(EasSyncMsg* self)
{
	EasSyncMsgPrivate *priv = self->priv;
    g_debug ("eas_sync_msg_getSyncKey +-");
	return priv->sync_key;
}

gboolean
eas_sync_msg_get_more_available (EasSyncMsg *self)
{
	EasSyncMsgPrivate *priv = self->priv;
	g_debug ("eas_sync_msg_get_more_available +-");
	return priv->more_available;

}

