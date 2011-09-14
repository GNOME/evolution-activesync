/*
 * ActiveSync client library for email access
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libxml/globals.h>

#include "eas-attachment.h"

G_DEFINE_TYPE (EasAttachment, eas_attachment, G_TYPE_OBJECT);

const gchar *attachment_separator = "\n";


static void
eas_attachment_init (EasAttachment *object)
{
	/* initialization code */
	object->file_reference = NULL;
	object->estimated_size = 0;
	object->display_name = NULL;
}

static void
eas_attachment_finalize (GObject *object)
{
	EasAttachment *self = (EasAttachment*) object;
	g_debug ("eas_attachment_finalize++");
	/* deinitalization code */
	xmlFree (self->file_reference);
	xmlFree (self->display_name);
	G_OBJECT_CLASS (eas_attachment_parent_class)->finalize (object);
	g_debug ("eas_attachment_finalize--");
}

static void
eas_attachment_class_init (EasAttachmentClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_attachment_finalize;
}

EasAttachment *
eas_attachment_new()
{
	EasAttachment *object = NULL;
	g_debug ("eas_attachment_new++");


	object = g_object_new (EAS_TYPE_ATTACHMENT , NULL);

	g_debug ("eas_attachment_new--");

	return object;
}


gboolean
eas_attachment_serialise (EasAttachment *attachment, gchar **result)
{
	g_debug ("eas_attachment_serialise++");

	*result = g_strdup_printf ("%s%s%s%s%d",
				   attachment->file_reference,
				   attachment_separator,
				   attachment->display_name,
				   attachment_separator,
				   attachment->estimated_size);

	if (!*result) {
		g_debug ("eas_attachment_serialise--");
		return FALSE;
	}

	g_debug ("eas_attachment_serialise--");
	return TRUE;
}
