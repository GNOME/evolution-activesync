/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */
#include "eas-sync-msg.h"
#include "eas-update-calendar-req.h"



G_DEFINE_TYPE (EasUpdateCalendarReq, eas_update_calendar_req, EAS_TYPE_REQUEST_BASE);

#define EAS_UPDATE_CALENDAR_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_UPDATE_CALENDAR_REQ, EasUpdateCalendarReqPrivate))

struct _EasUpdateCalendarReqPrivate
{
	EasSyncMsg* sync_msg;
	guint64 account_id;
	gchar* sync_key;
	gchar* folder_id;
	GSList *serialised_calendar; 
};

static void
eas_update_calendar_req_init (EasUpdateCalendarReq *object)
{
	g_debug("eas_update_calendar_req_init++");
	/* initialization code */
	EasUpdateCalendarReqPrivate *priv;
	
	object->priv = priv = EAS_UPDATE_CALENDAR_REQ_PRIVATE(object);

	priv->sync_msg = NULL;
	priv->account_id = 0;
	priv->sync_key = NULL;
	priv->folder_id = NULL;
	priv->serialised_calendar = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_UPDATE_CALENDAR);

	g_debug("eas_update_calendar_req_init++");

	return;
}

static void
eas_update_calendar_req_finalize (GObject *object)
{
	g_debug("eas_update_calendar_req_finalize++");
	/* deinitalization code */
	EasUpdateCalendarReq *req = (EasUpdateCalendarReq *) object;
	EasUpdateCalendarReqPrivate *priv = req->priv;

	g_object_unref(priv->sync_msg);
	g_free (priv);
	req->priv = NULL;

	G_OBJECT_CLASS (eas_update_calendar_req_parent_class)->finalize (object);

	g_debug("eas_update_calendar_req_finalize--");	
}

static void
eas_update_calendar_req_class_init (EasUpdateCalendarReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;

	g_type_class_add_private (klass, sizeof (EasUpdateCalendarReqPrivate));	
	
	object_class->finalize = eas_update_calendar_req_finalize;

	g_debug("eas_update_calendar_req_class_init--");
}


// TODO - update this to take a GSList of serialised calendars? rem to copy the list
EasUpdateCalendarReq *eas_update_calendar_req_new(guint64 account_id, const gchar *sync_key, const gchar *folder_id, const GSList* serialised_calendar, EFlag *flag)
{
	g_debug("eas_update_calendar_req_new++");

	EasUpdateCalendarReq* self = g_object_new (EAS_TYPE_UPDATE_CALENDAR_REQ, NULL);
	EasUpdateCalendarReqPrivate *priv = self->priv;
	
	g_assert(sync_key);
	g_assert(folder_id);
	g_assert(serialised_calendar);
	
	priv->sync_key = g_strdup(sync_key);
	priv->folder_id = g_strdup(folder_id);
	priv->serialised_calendar = serialised_calendar;
	priv->account_id = account_id;

	eas_request_base_SetFlag(&self->parent_instance, flag);

	g_debug("eas_update_calendar_req_new--");
	return self;	
}

void eas_update_calendar_req_Activate(EasUpdateCalendarReq *self)
{
	EasUpdateCalendarReqPrivate *priv = self->priv;
	xmlDoc *doc;
	//GSList *update_calendars = NULL;   // sync msg expects a list, we'll just have a list of one (for now)
	//update_calendars = g_slist_append(update_calendars, priv->serialised_calendar);
	
	g_debug("eas_update_calendar_req_Activate++");
	//create sync msg object
	priv->sync_msg = eas_sync_msg_new (priv->sync_key, priv->account_id, priv->folder_id, EAS_ITEM_CALENDAR);

	g_debug("build messsage");
	//build request msg
	doc = eas_sync_msg_build_message (priv->sync_msg, FALSE, NULL, priv->serialised_calendar, NULL);
	
	g_debug("send message");
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self);

	g_debug("eas_update_calendar_req_Activate--");		

	return;
}


void eas_update_calendar_req_MessageComplete(EasUpdateCalendarReq *self, xmlDoc* doc)
{
	g_debug("eas_update_calendar_req_MessageComplete++");	

	EasUpdateCalendarReqPrivate *priv = self->priv;

	eas_sync_msg_parse_reponse (priv->sync_msg, doc);

	xmlFree(doc);
	
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));

	g_debug("eas_update_calendar_req_MessageComplete--");	
}

void eas_update_calendar_req_ActivateFinish (EasUpdateCalendarReq* self, gchar** ret_sync_key, GError **error)
{
	g_debug("eas_update_calendar_req_ActivateFinish++");
	
	EasUpdateCalendarReqPrivate *priv = self->priv;

	*ret_sync_key = g_strdup(eas_sync_msg_get_syncKey(priv->sync_msg));

	// lrm TODO fill in the error
	
	g_debug("eas_update_calendar_req_ActivateFinish--");	
}



