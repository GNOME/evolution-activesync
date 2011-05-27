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

#include <stdio.h>

#include "eas-send-email-req.h"
#include "eas-send-email-msg.h"



G_DEFINE_TYPE (EasSendEmailReq, eas_send_email_req, EAS_TYPE_REQUEST_BASE);

#define EAS_SEND_EMAIL_REQ_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_SEND_EMAIL_REQ, EasSendEmailReqPrivate))

struct _EasSendEmailReqPrivate
{
	EasSendEmailMsg *send_email_msg;
	guint64 account_id;
	gchar *mime_string;	 
};

static void
eas_send_email_req_init (EasSendEmailReq *object)
{
	g_debug("eas_send_email_req_init++");
	/* initialization code */
	EasSendEmailReqPrivate *priv;

	object->priv = priv = EAS_SEND_EMAIL_REQ_PRIVATE(object);

	priv->account_id = 0;   // assuming 0 not a valid account id
	priv->send_email_msg = NULL;
	priv->mime_string = NULL;

	eas_request_base_SetRequestType (&object->parent_instance, 
	                                 EAS_REQ_SEND_EMAIL);

	g_debug("eas_send_email_req_init--");	
}

static void
eas_send_email_req_finalize (GObject *object)
{
	/* deinitalization code */
	EasSendEmailReq *req = (EasSendEmailReq *)object;
	
	EasSendEmailReqPrivate *priv = req->priv;	
	g_free(priv->mime_string);
	g_object_unref(priv->send_email_msg);
	g_free (priv);
	req->priv = NULL;

	G_OBJECT_CLASS (eas_send_email_req_parent_class)->finalize (object);
}

static void
eas_send_email_req_class_init (EasSendEmailReqClass *klass)
{
	g_debug("eas_send_email_req_class_init++");
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass* parent_class = EAS_REQUEST_BASE_CLASS (klass);
	
	// get rid of warnings about above 2 lines
	void *temp = (void*)object_class;
	temp = (void*)parent_class;

	g_type_class_add_private (klass, sizeof (EasSendEmailReqPrivate));	
	
	object_class->finalize = eas_send_email_req_finalize;

	g_debug("eas_send_email_req_class_init--");
}

EasSendEmailReq *
eas_send_email_req_new()
{
	g_debug("eas_send_email_req_new++");	
	
	EasSendEmailReq *object = NULL;

	object = g_object_new (EAS_TYPE_SEND_EMAIL_REQ, NULL);

	g_debug("eas_send_email_req_new--");	
	
	return object;
}

// uses the message object to build xml and sends it to the connection object
void 
eas_send_email_req_Activate(EasSendEmailReq *self, guint64 account_id, EFlag *flag, const gchar* client_id, const gchar* mime_file, EasItemType type)
{
	EasSendEmailReqPrivate* priv = self->priv;
	xmlDoc *doc;
	FILE *file = NULL;
	
	// store flag in base (doesn't set the flag as in signal the waiters)
	g_debug("eas_send_email_req_Activate++");
	eas_request_base_SetFlag(&self->parent_instance, flag);
	
	priv->account_id = account_id;

	// open file containing mime data to be sent:
	file = fopen(mime_file, "r");  // mime file can be read as text (attachments will be base 64 encoded)
	if(file == NULL)
	{
		// TODO - how do we propogate errors?
	}

	// read data from the file to mime_string 
	// ..and escape any xml characters; libxml2 may do this when we construct the xml??
	
	guint64 size;
	gchar * buffer;
	size_t result;

	// obtain file size:
	fseek (file , 0 , SEEK_END);
	size = ftell (file);
	g_debug("file size = %d", size);
	rewind (file);

	// allocate memory to contain the whole file:
	buffer = (gchar*) g_malloc0 (sizeof(gchar)*size);
	if (buffer == NULL) 
	{
	// TODO - how do we propogate errors?		
	}

	// copy the file into the buffer:
	result = fread (buffer,1,size,file);
	if (result != size) 
	{
	// TODO - how do we propogate errors?
	}
	
	fclose (file);
	
	g_debug("create msg object");
	//create msg object
	priv->send_email_msg = eas_send_email_msg_new(account_id, client_id, priv->mime_string);	

	g_debug("build messsage");
	//build request msg
	doc = eas_send_email_msg_build_message (priv->send_email_msg);

	g_debug("send message");
	eas_connection_send_request(eas_request_base_GetConnection (&self->parent_instance), "SendMail", doc, self);
	
	g_debug("eas_send_email_req_Activate++");

	return;
}


void 
eas_send_email_req_MessageComplete(EasSendEmailReq *self, xmlDoc* doc)
{
	EasSendEmailReqPrivate *priv = self->priv;
	
	g_debug("eas_send_email_req_MessageComplete++");

	eas_send_email_msg_parse_reponse(priv->send_email_msg, doc);

	xmlFree(doc);

	// signal daemon we're done
	e_flag_set(eas_request_base_GetFlag (&self->parent_instance));	

	g_debug("eas_send_email_req_MessageComplete--");

	return;
}

