/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eas-sync-msg.h"
#include "eas-update-email-req.h"

G_DEFINE_TYPE (EasUpdateEmailReq, eas_update_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_UPDATE_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_UPDATE_EMAIL_REQ, EasUpdateEmailReqPrivate))

struct _EasUpdateEmailReqPrivate
{
	EasSyncMsg* sync_msg;
	guint64 account_id;
	gchar* sync_key;
	gchar* folder_id;
	gchar* serialised_email; 
};

static void
eas_update_email_req_init (EasUpdateEmailReq *object)
{
	g_debug("eas_update_email_req_init++");
	/* initialization code */
	EasUpdateEmailReqPrivate *priv;
	
	object->priv = priv = EAS_UPDATE_EMAIL_REQ_PRIVATE(object);

	priv->sync_msg = NULL;
	priv->account_id = 0;
	priv->sync_key = NULL;
	priv->folder_id = NULL;
	priv->serialised_email = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_UPDATE_MAIL);

	g_debug("eas_update_email_req_init++");

	return;
	
}

static void
eas_update_email_req_finalize (GObject *object)
{
	g_debug("eas_update_email_req_finalize++");
	/* deinitalization code */
	EasUpdateEmailReq *req = (EasUpdateEmailReq *) object;
	EasUpdateEmailReqPrivate *priv = req->priv;

	g_object_unref(priv->sync_msg);
	g_free (priv);
	req->priv = NULL;

	G_OBJECT_CLASS (eas_update_email_req_parent_class)->finalize (object);

	g_debug("eas_update_email_req_finalize--");	
}

static void
eas_update_email_req_class_init (EasUpdateEmailReqClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);

	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;

	g_type_class_add_private (klass, sizeof (EasUpdateEmailReqPrivate));	
	
	object_class->finalize = eas_update_email_req_finalize;

	g_debug("eas_update_email_req_class_init--");
}

EasUpdateEmailReq *eas_update_email_req_new(guint64 account_id, const gchar *sync_key, const gchar *folder_id, const gchar *serialised_email, EFlag *flag)
{
	g_debug("eas_update_email_req_new++");

	EasUpdateEmailReq* self = g_object_new (EAS_TYPE_UPDATE_EMAIL_REQ, NULL);
	EasUpdateEmailReqPrivate *priv = self->priv;
	
	g_assert(sync_key);
	g_assert(folder_id);
	g_assert(serialised_email);
	
	priv->sync_key = g_strdup(sync_key);
	priv->folder_id = g_strdup(folder_id);
	priv->serialised_email = g_strdup(serialised_email);
	priv->account_id = account_id;

	eas_request_base_SetFlag(&self->parent_instance, flag);

	g_debug("eas_update_email_req_new--");
	return self;	
}

void eas_update_email_req_Activate(EasUpdateEmailReq *self)
{
	EasUpdateEmailReqPrivate *priv = self->priv;
	xmlDoc *doc;
	GSList *update_emails = NULL;   // sync msg expects a list, we'll just have a list of one (for now)
	update_emails = g_slist_append(update_emails, priv->serialised_email);
	
	g_debug("eas_delete_email_req_Activate++");
	//create sync msg object
	priv->sync_msg = eas_sync_msg_new (priv->sync_key, priv->account_id, priv->folder_id, EAS_ITEM_MAIL);

	g_debug("build messsage");
	//build request msg
	doc = eas_sync_msg_build_message (priv->sync_msg, FALSE, NULL, update_emails, NULL);
	
	g_debug("send message");
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "Sync", doc, self);

	g_debug("eas_update_email_req_Activate--");		

	return;
}


void eas_update_email_req_MessageComplete(EasUpdateEmailReq *self, xmlDoc* doc)
{
	// TODO
}

void eas_update_email_req_ActivateFinish (EasUpdateEmailReq* self, gchar** ret_sync_key)
{
	// TODO
}

