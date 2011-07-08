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
#include "utils.h"

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
	gchar est_size[MAX_LEN_OF_INT32_AS_STRING] = "";
	gchar *strings[4] = {0, 0, 0, 0};
	g_debug ("eas_attachment_serialise++");
	g_assert (attachment->estimated_size);

	snprintf (est_size, sizeof (est_size) / sizeof (est_size[0]), "%d", attachment->estimated_size);

	strings[0] = (gchar*) attachment->file_reference;
	strings[1] = (gchar*) attachment->display_name;
	strings[2] = est_size;

	*result = g_strjoinv (attachment_separator, strings);

	if (!*result) {
		g_debug ("eas_attachment_serialise--");
		return FALSE;
	}

	g_debug ("eas_attachment_serialise--");
	return TRUE;
}

gboolean
eas_attachment_deserialise (EasAttachment *attachment, const gchar *data)
{
	gboolean ret = TRUE;
	gchar *from = (gchar*) data;
	gchar *est_size = NULL;

	g_debug ("eas_attachment_deserialise++");
	g_assert (attachment);
	g_assert (data);

	// file_reference
	if (attachment->file_reference != NULL) { //just in case
		g_free (attachment->file_reference);
	}
	attachment->file_reference = (xmlChar*) get_next_field (&from, attachment_separator);
	if (!attachment->file_reference) {
		ret = FALSE;
		goto cleanup;
	}
	g_debug ("file_reference = %s", attachment->file_reference);

	// display_name
	if (attachment->display_name != NULL) {
		g_free (attachment->display_name);
	}
	attachment->display_name = (xmlChar*) get_next_field (&from, attachment_separator);
	if (!attachment->display_name) {
		ret = FALSE;
		goto cleanup;
	}
	g_debug ("display name = %s", attachment->display_name);

	//estimated_size
	est_size = get_next_field (&from, attachment_separator);
	if (!est_size) {
		ret = FALSE;
		goto cleanup;
	}
	if (strlen (est_size)) {
		attachment->estimated_size = atoi (est_size);
		g_free (est_size);
	}
	g_debug ("estimated_size = %d", attachment->estimated_size);

cleanup:
	if (!ret) {
		g_free (attachment->file_reference);
		attachment->file_reference = NULL;
		g_free (attachment->display_name);
		attachment->display_name = NULL;
		attachment->estimated_size = 0;
	}

	g_debug ("eas_attachment_deserialise--");

	return ret;
}


guint
eas_attachment_serialised_length (EasAttachment *attachment)
{
	guint len = 0;
	gchar est_size[MAX_LEN_OF_INT32_AS_STRING] = "";

	// file_reference:
	g_assert (attachment->file_reference);
	len += strlen ( (gchar*) attachment->file_reference) + 1; // null-terminate allows for separator
	// display_name:
	if (attachment->display_name) { // optional field
		len += strlen ( (gchar*) attachment->display_name) + 1;
	} else {
		len += 1;   // just separator
	}
	// estimated_size:
	g_assert (attachment->estimated_size);
	snprintf (est_size, sizeof (est_size) / sizeof (est_size[0]), "%d", attachment->estimated_size);

	len += strlen (est_size) + 1;   // no separator at end, allows for null terminate

	g_debug ("eas_attachment_serialised_length returning %d", len);
	return len;

}
