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

struct _EasRequestBasePrivate
{
    EasRequestType requestType;
    struct _EasConnection* connection;
    SoupMessage *soup_message;
    EFlag *flag;
	DBusGMethodInvocation *context;
	EasMail *dbus_interface;
};

#define EAS_REQUEST_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_REQUEST_BASE, EasRequestBasePrivate))



G_DEFINE_TYPE (EasRequestBase, eas_request_base, G_TYPE_OBJECT);

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
    priv->connection = NULL;
	priv->dbus_interface = NULL;

    g_debug ("eas_request_base_init--");
}

static void
eas_request_base_dispose (GObject *object)
{
	EasRequestBase *req = (EasRequestBase *)object;
	EasRequestBasePrivate *priv = req->priv;

	g_debug ("eas_request_base_dispose++");
	if(priv->connection)
	{
		g_debug("not unrefing connection");
        // TODO Fix the unreff count.
		// g_object_unref(priv->connection);
        // priv->connection = NULL;
	}
    G_OBJECT_CLASS (eas_request_base_parent_class)->dispose (object);
	g_debug ("eas_request_base_dispose--");
}

static void
eas_request_base_finalize (GObject *object)
{
	g_debug ("eas_request_base_finalize++");
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

    g_debug ("eas_request_base_class_init--");
}

EasRequestType
eas_request_base_GetRequestType (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;

    return priv->requestType;
}

void
eas_request_base_SetRequestType (EasRequestBase* self, EasRequestType type)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetRequestType++");
    priv->requestType = type;
    g_debug ("eas_request_base_SetRequestType--");
}

struct _EasConnection*
eas_request_base_GetConnection (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_GetConnection++ %lx", (unsigned long)priv->connection );
    return priv->connection;
}

void
eas_request_base_SetConnection (EasRequestBase* self, struct _EasConnection* connection)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetConnection++");
    priv->connection = connection;
    g_debug ("eas_request_base_SetConnection--");
}

// lrm
void
eas_request_base_SetInterfaceObject (EasRequestBase* self, EasMail *dbus_interface)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug ("eas_request_base_SetInterfaceObject++");
    priv->dbus_interface = dbus_interface;
    g_debug ("eas_request_base_SetInterfaceObject--");
}

EasMail* 
eas_request_base_GetInterfaceObject (EasRequestBase* self)
{
    EasRequestBasePrivate *priv = self->priv;
    return priv->dbus_interface;
}


SoupMessage *
eas_request_base_GetSoupMessage(EasRequestBase *self)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_SoupMessage++ %lx", (unsigned long)priv->soup_message );
    return priv->soup_message;
}

void
eas_request_base_SetSoupMessage(EasRequestBase *self, SoupMessage *soup_message)
{
    EasRequestBasePrivate *priv = self->priv;
    g_debug("eas_request_base_SetSoupMessage++");
    priv->soup_message = soup_message;
    g_debug("eas_request_base_SetSoupMessage--");
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
