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
#include <stdlib.h>
#include <string.h>
#include <libxml/globals.h>

#include "eas-email-info.h"
#include "eas-attachment.h"

G_DEFINE_TYPE (EasEmailInfo, eas_email_info, G_TYPE_OBJECT);

const gchar *sep = "\n";

static void
eas_email_info_init (EasEmailInfo *object)
{
	g_debug ("eas_email_info_init++");
	/* initialization code */
	object->server_id = NULL;
	object->headers = NULL;
	object->attachments = NULL;
	object->categories = NULL;
	object->status = NULL;
	object->flags = 0;
	g_debug ("eas_email_info_init--");
}

static void
eas_email_free_header (EasEmailHeader *header)
{
	g_free (header->name);
	g_free (header->value);
	g_free (header);
}

static void
eas_email_info_finalize (GObject *object)
{
	EasEmailInfo *self = (EasEmailInfo*) object;

	g_debug ("eas_email_info_finalize++");
	/* deinitalization code */
	g_free (self->server_id);
	g_free (self->status);

	g_slist_foreach (self->headers, (GFunc) eas_email_free_header, NULL);
	g_slist_free (self->headers);

	g_slist_foreach (self->attachments, (GFunc) g_object_unref, NULL);	// TODO - these should be done in dispose?
	g_slist_free (self->attachments); // list of EasAttachments

	g_slist_foreach (self->categories, (GFunc) xmlFree, NULL);
	g_slist_free (self->categories);

	G_OBJECT_CLASS (eas_email_info_parent_class)->finalize (object);
	g_debug ("eas_email_info_finalize--");
}

static void
eas_email_info_class_init (EasEmailInfoClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_email_info_finalize;
}

EasEmailInfo *
eas_email_info_new()
{
	EasEmailInfo *object = NULL;
	g_debug ("eas_email_info_new++");

	object = g_object_new (EAS_TYPE_EMAIL_INFO , NULL);

	g_debug ("eas_email_info_new--");

	return object;
}


gboolean
eas_email_info_serialise (EasEmailInfo* self, gchar **result)
{
	GString *ser;
	gboolean ret = TRUE;
	gchar *temp = NULL;
	guint list_len = 0;
	GSList *l = NULL;
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	gchar *category = NULL;

	g_debug ("eas_email_info_serialise++");

	// serialise everything:
	//server_id
	g_debug ("serialising serverid");
	ser = g_string_new (self->server_id);
	g_string_append (ser, sep);

	//headers
	g_debug ("serialising headers");
	list_len = g_slist_length (self->headers);
	g_string_append_printf (ser, "%d\n", list_len);

	for (l = self->headers; l != NULL; l = g_slist_next (l)) {
		header = l->data;
		g_string_append_printf (ser, "%s\n%s\n", header->name, header->value);
	}

	//attachments
	g_debug ("serialising attachments");
	list_len = g_slist_length (self->attachments);
	g_string_append_printf (ser, "%d\n", list_len);

	for (l = self->attachments; l != NULL; l = g_slist_next (l)) {
		attachment = l->data;

		if (!eas_attachment_serialise (attachment, &temp)) {
			ret = FALSE;
		} else {
			g_string_append (ser, temp);
			g_free (temp);
			temp = NULL;
			g_string_append (ser, sep);
		}
	}

	//flags
	g_debug ("serialising flags");
	g_string_append_printf (ser, "%d\n", self->flags);

	//categories
	g_debug ("serialising categories");
	list_len = g_slist_length (self->categories);
	g_string_append_printf (ser, "%d\n", list_len);
	for (l = self->categories; l != NULL; l = g_slist_next (l)) {
		category = l->data;
		g_string_append_printf (ser, "%s\n", category);
	}
	// estimated size, date received
	g_string_append_printf (ser, "%zu\n%ld\n%d\n", self->estimated_size, self->date_received, self->importance);

	// status
	g_debug ("serialising status %s", self->status);
	g_string_append_printf (ser, "%s", (self->status ? : ""));

	if (ret) {
		*result = ser->str;
		g_string_free (ser, FALSE);
	} else {
		g_debug ("failed!");
		g_string_free (ser, TRUE);
		*result = NULL;
	}

	g_debug ("eas_email_info_serialise--");
	return ret;
}

gboolean
eas_email_info_deserialise (EasEmailInfo* self, const gchar *data)
{
	// TODO proper error handling - eg deal with get_next_field returning null
	gboolean ret = FALSE;
	guint list_len = 0, i = 0;
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	GSList *headers = NULL;
	GSList *attachments = NULL;
	GSList *categories = NULL;
	gchar **strv;
	int strvlen;
	int idx = 0;

	g_debug ("eas_email_info_deserialise++");
	g_assert (self);
	g_assert (data);

	strv = g_strsplit (data, sep, 0);	// split into array of strings
	if (!strv)
		goto out;

	strvlen = g_strv_length (strv);

	idx = 0;
	self->server_id = strv[idx++];
	g_debug ("server_id = %s", self->server_id);

	//headers
	if (!strv[idx]) {
		// This is permitted; for deleted mail we get *only* the ID
		ret = TRUE;
		goto out;
	}
	list_len = atoi (strv[idx]);
	g_free (strv[idx++]);
	g_debug ("%d headers", list_len);

	if (strvlen < idx + (2 * list_len)) {
		g_warning ("More headers than actual data");
		goto out;
	}
	for (i = 0; i < list_len; i++) {
		header = g_malloc0 (sizeof (EasEmailHeader));
		header->name = strv[idx++];
		header->value = strv[idx++];
		headers = g_slist_append (headers, header);
	}
	self->headers = headers;

	//attachments
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	list_len = atoi (strv[idx]);
	g_free (strv[idx++]);
	g_debug ("%d attachments", list_len);

	if (strvlen < idx + (3 * list_len)) {
		g_warning ("More attachments than actual data");
		goto out;
	}
	for (i = 0; i < list_len; i++) {
		attachment = eas_attachment_new ();
		attachment->file_reference = (xmlChar *) strv[idx++];
		attachment->display_name = (xmlChar *) strv[idx++];
		attachment->estimated_size = atoi (strv[idx]);
		g_free (strv[idx++]);
		attachments = g_slist_append (attachments, attachment);
	}
	self->attachments = attachments;

	//flags
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	self->flags = atoi (strv[idx]);
	g_free (strv[idx++]);
	g_debug ("flags = %x", self->flags);

	//categories
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	list_len = atoi (strv[idx]);
	g_free (strv[idx++]);
	g_debug ("%d categories", list_len);

	if (strvlen < idx + list_len) {
		g_warning ("More categories than actual data");
		goto out;
	}
	for (i = 0; i < list_len; i++) {
		categories = g_slist_append (categories, strv[idx++]);
	}
	self->categories = categories;

	//estimated_size
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	self->estimated_size = strtoul (strv[idx], NULL, 10);
	g_free (strv[idx++]);
	g_debug ("estimated size = %zu", self->estimated_size);

	//date_received
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	self->date_received = strtoul (strv[idx], NULL, 10);
	g_free (strv[idx++]);
	g_debug ("date received = %ld", self->date_received);

	//importance
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	self->importance = strtoul (strv[idx], NULL, 10);
	g_free (strv[idx++]);
	g_debug ("importance = %d", self->importance);

	//status
	if (!strv[idx]) {
		g_warning ("Insufficient data in eas_email_info_serialise");
		goto out;
	}
	self->status = strv[idx++];
	g_debug ("status = %s", self->status);

	ret = TRUE;
out:
	while (strv[idx])
		g_free (strv[idx++]);
	g_free (strv[idx]);
	g_free (strv);

	if (!ret) {
		g_warning ("failed!");
	}

	g_debug ("eas_email_info_deserialise--");
	return ret;
}

