/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */
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

#include "eas-msg-base.h"

struct _EasMsgBasePrivate {
	EasMsgType messageType;
	GObject* accountsObject;
};

#define EAS_MSG_BASE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), EAS_TYPE_MSG_BASE, EasMsgBasePrivate))


G_DEFINE_TYPE (EasMsgBase, eas_msg_base, G_TYPE_OBJECT);

static void
eas_msg_base_init (EasMsgBase *object)
{
	EasMsgBasePrivate *priv;
	g_debug ("eas_msg_base_init++");
	object->priv = priv = EAS_MSG_BASE_PRIVATE (object);

	priv->messageType = EAS_MSG_BASE;
	priv->accountsObject = NULL;

	g_debug ("eas_msg_base_init--");
}

static void
eas_msg_base_finalize (GObject *object)
{
	g_debug ("eas_msg_base_finalize++");
	G_OBJECT_CLASS (eas_msg_base_parent_class)->finalize (object);
	g_debug ("eas_msg_base_finalize--");
}

static void
eas_msg_base_class_init (EasMsgBaseClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_debug ("eas_msg_base_class_init++");

	g_type_class_add_private (klass, sizeof (EasMsgBasePrivate));

	object_class->finalize = eas_msg_base_finalize;

	g_debug ("eas_msg_base_class_init--");
}
