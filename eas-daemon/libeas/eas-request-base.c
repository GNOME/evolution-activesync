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

#include "eas-request-base.h"

struct _EasRequestBasePrivate {
	EasRequestType requestType;
	struct _EasConnection* connection;
	SoupMessage *soup_message;
	EFlag *flag;
	DBusGMethodInvocation *context;
	EasInterfaceBase *dbus_interface;
	gboolean outgoing_progress;		// whether the progress updates are for outgoing/incoming data
	gchar *request_owner;			// dbus sender of message
	guint request_id;			// passed back with progress signal
	gboolean cancelled;			// whether the request has been cancelled
	guint data_length_so_far;	// amount of data received/sent so far 
	guint data_size;			// total size of response/request data
	gboolean use_multipart;
	guchar* wbxml;
	gsize wbxml_length;
};

#define EAS_REQUEST_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_REQUEST_BASE, EasRequestBasePrivate))

G_DEFINE_TYPE (EasRequestBase, eas_request_base, G_TYPE_OBJECT);

void _eas_request_base_GotChunk (EasRequestBase *self, SoupMessage *msg, SoupBuffer *chunk);

guchar* 
eas_request_base_GetWbxmlFromChunking (EasRequestBase *self)
{
	return EAS_REQUEST_BASE_PRIVATE (self)->wbxml;
}

gsize 
eas_request_base_GetWbxmlFromChunkingSize (EasRequestBase *self)
{
	return EAS_REQUEST_BASE_PRIVATE (self)->wbxml_length;
}


void 
eas_request_base_SetWbxmlFromChunking (EasRequestBase *self, guchar* wbxml, gsize wbxml_length)
{
	EasRequestBasePrivate *priv = EAS_REQUEST_BASE_PRIVATE (self);

	g_free (priv->wbxml);
	priv->wbxml_length = 0;
	
	priv->wbxml = g_malloc0 (wbxml_length);
	if (priv->wbxml) {
		memcpy (priv->wbxml, wbxml, wbxml_length);
		priv->wbxml_length = wbxml_length;
	}
}


static void
eas_request_base_init (EasRequestBase *object)
{
	EasRequestBasePrivate *priv;

	object->priv = priv = EAS_REQUEST_BASE_PRIVATE (object);

	g_debug ("eas_request_base_init++");

	priv->requestType = EAS_REQ_BASE;
	priv->connection = NULL;
	priv->soup_message = NULL;
	priv->flag = NULL;
	priv->context = NULL;
	priv->dbus_interface = NULL;
	priv->data_length_so_far = 0;
	priv->data_size = 0;
	priv->request_id = 0;
	priv->request_owner = NULL;
	priv->cancelled = FALSE;
	priv->use_multipart = FALSE;
	priv->wbxml = NULL;

	g_debug ("eas_request_base_init--");
}

static void
eas_request_base_dispose (GObject *object)
{
	EasRequestBase *req = (EasRequestBase *) object;
	EasRequestBasePrivate *priv = req->priv;

	g_debug ("eas_request_base_dispose++");
	if(priv->connection){
		g_debug("not unrefing connection");
        // TODO Fix the unref count.
		// g_object_unref(priv->connection);
		// priv->connection = NULL;
	}
	G_OBJECT_CLASS (eas_request_base_parent_class)->dispose (object);
	g_debug ("eas_request_base_dispose--");
}

static void
eas_request_base_finalize (GObject *object)
{
	EasRequestBasePrivate *priv = EAS_REQUEST_BASE_PRIVATE (object);
	
	g_debug ("eas_request_base_finalize++");
	g_free (priv->wbxml);
	g_free (priv->request_owner);
	G_OBJECT_CLASS (eas_request_base_parent_class)->finalize (object);
	g_debug ("eas_request_base_finalize--");
}

static void
eas_request_base_class_init (EasRequestBaseClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_request_base_class_init++");
	g_type_class_add_private (klass, sizeof (EasRequestBasePrivate));

	object_class->finalize = eas_request_base_finalize;
	object_class->dispose = eas_request_base_dispose;

	klass->do_MessageComplete = NULL;
	klass->do_GotChunk = _eas_request_base_GotChunk;

	g_debug ("eas_request_base_class_init--");
}

EasRequestType
eas_request_base_GetRequestType (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->requestType;
}

void
_eas_request_base_GotChunk (EasRequestBase *self, 
                            SoupMessage *msg, 
                            SoupBuffer *chunk)
{
	g_debug ("_eas_request_base_GotChunk+-");
}

void
eas_request_base_GotChunk (EasRequestBase *self, 
                           SoupMessage *msg, 
                           SoupBuffer *chunk)
{
	g_debug ("eas_request_base_GotChunk+-");
	EAS_REQUEST_BASE_GET_CLASS (self)->do_GotChunk (self, msg, chunk);
}

gboolean
eas_request_base_MessageComplete (EasRequestBase *self,
				  xmlDoc* doc,
				  GError* error_in)
{
	g_return_val_if_fail (EAS_IS_REQUEST_BASE (self), TRUE);

	g_debug ("eas_request_base_MessageComplete+-");

	return EAS_REQUEST_BASE_GET_CLASS (self)->do_MessageComplete (self, doc, error_in);
}

gboolean
eas_request_base_SendRequest (EasRequestBase* self,
			      const gchar* cmd,
			      xmlDoc *doc,
                  gboolean highpriority,
			      GError **error)
{
	EasRequestBasePrivate *priv = self->priv;

	g_debug ("eas_request_base_SendRequest");

	return eas_connection_send_request (priv->connection,
					    cmd,
					    doc, // full transfer
					    self,
	                    highpriority,
					    error);
}

void
eas_request_base_SetRequestType (EasRequestBase* self, EasRequestType type)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetRequestType++");
	priv->requestType = type;
	g_debug ("eas_request_base_SetRequestType--");
}

guint
eas_request_base_GetRequestId (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->request_id;
}

void
eas_request_base_SetRequestId (EasRequestBase* self, guint request_id)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->request_id = request_id;

	return ;
}

const gchar *
eas_request_base_GetRequestOwner (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->request_owner;
}

void
eas_request_base_SetRequestOwner (EasRequestBase* self, gchar *request_owner)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->request_owner = request_owner;

	return ;
}

gboolean 
eas_request_base_IsCancelled (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->cancelled;
	
}

void 
eas_request_base_Cancelled (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->cancelled = TRUE;

	return;
}


gboolean
eas_request_base_GetRequestProgressDirection (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->outgoing_progress;
}

void
eas_request_base_SetRequestProgressDirection (EasRequestBase* self, gboolean outgoing_progress)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->outgoing_progress = outgoing_progress;

	return ;
}

guint
eas_request_base_GetDataSize (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->data_size;
}

void
eas_request_base_SetDataSize (EasRequestBase* self, guint size)
{
	EasRequestBasePrivate *priv = self->priv;

	g_debug ("eas_request_base_SetDataSize++");

	priv->data_size = size;

	g_debug ("eas_request_base_SetDataSize--");

	return;
}

guint
eas_request_base_GetDataLengthSoFar (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;

	return priv->data_length_so_far;
}

void
eas_request_base_UpdateDataLengthSoFar (EasRequestBase* self, guint length)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->data_length_so_far += length;

	return;
}

void
eas_request_base_SetDataLengthSoFar (EasRequestBase* self, guint length)
{
	EasRequestBasePrivate *priv = self->priv;

	priv->data_length_so_far = length;

	return;
}

struct _EasConnection*
eas_request_base_GetConnection (EasRequestBase* self) {
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_GetConnection++ %p", priv->connection);
	return priv->connection;
}

void
eas_request_base_SetConnection (EasRequestBase* self, struct _EasConnection* connection)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetConnection++ [%p]", connection);
	priv->connection = connection;
	g_debug ("eas_request_base_SetConnection--");
}


void
eas_request_base_SetInterfaceObject (EasRequestBase* self, EasInterfaceBase *dbus_interface)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetInterfaceObject++");
	priv->dbus_interface = dbus_interface;
	g_debug ("eas_request_base_SetInterfaceObject--");
}

EasInterfaceBase*
eas_request_base_GetInterfaceObject (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;
	return priv->dbus_interface;
}


SoupMessage *
eas_request_base_GetSoupMessage (EasRequestBase *self)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SoupMessage++ %lx", (unsigned long) priv->soup_message);
	return priv->soup_message;
}

void
eas_request_base_SetSoupMessage (EasRequestBase *self, SoupMessage *soup_message)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetSoupMessage++");
	priv->soup_message = soup_message;
	g_debug ("eas_request_base_SetSoupMessage--");
}

EFlag *
eas_request_base_GetFlag (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_GetFlag+-");
	return priv->flag;
}

void
eas_request_base_SetFlag (EasRequestBase* self, EFlag* flag)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetFlag++");
	priv->flag = flag;
	g_debug ("eas_request_base_SetFlag--");
}

DBusGMethodInvocation*
eas_request_base_GetContext (EasRequestBase* self)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_GetContext+-");
	return priv->context;
}

void
eas_request_base_SetContext (EasRequestBase* self, DBusGMethodInvocation* context)
{
	EasRequestBasePrivate *priv = self->priv;
	g_debug ("eas_request_base_SetContext++");
	priv->context = context;
	g_debug ("eas_request_base_SetContext--");
}

gboolean
eas_request_base_UseMultipart (EasRequestBase* self)
{
	return self->priv->use_multipart;
}
void eas_request_base_Set_UseMultipart (EasRequestBase* self, gboolean use_multipart)
{
	self->priv->use_multipart = use_multipart;
}
