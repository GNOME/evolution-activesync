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
#include "utils.h"

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
    GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    // TODO better way to get rid of warnings about above 2 lines?
    void *temp = (void*) object_class;
    temp = (void*) parent_class;

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

	// serialise everything:
	//server_id
	g_debug("serialising serverid");
	ser = g_string_new (self->server_id);
	g_string_append (ser, sep);

	//headers
	g_debug("serialising headers");
	list_len = g_slist_length(self->headers);
	g_string_append_printf (ser, "%d\n", list_len);

	for (l = self->headers; l != NULL; l = g_slist_next (l)) {
		header = l->data;
		g_string_append_printf (ser, "%s\n%s\n", header->name, header->value);
	}

	//attachments
	g_debug("serialising attachments");
	list_len = g_slist_length(self->attachments);
	g_string_append_printf (ser, "%d\n", list_len);

	for (l = self->attachments; l != NULL; l = g_slist_next (l)) {
		attachment = l->data;

		if (!eas_attachment_serialise(attachment, &temp)) {
			ret = FALSE;
		} else {
			g_string_append (ser, temp);
			g_free(temp);
			temp = NULL;
			g_string_append (ser, sep);
		}
	}

	//flags
	g_debug("serialising flags");
	g_string_append_printf (ser, "%d\n", self->flags);

	//categories
	g_debug("serialising categories");
	list_len = g_slist_length(self->categories);
	g_string_append_printf (ser, "%d\n", list_len);
	for (l = self->categories; l != NULL; l = g_slist_next (l)) {
		category = l->data;
		g_string_append_printf (ser, "%s\n", category);
	}
	// estimated size, date received
	g_string_append_printf(ser, "%zu\n%ld\n%d", self->estimated_size, self->date_received, self->importance);

	if (ret) {
		*result = ser->str;
		g_string_free (ser, FALSE);
	} else {
		g_debug("failed!");
		g_string_free (ser, TRUE);
		*result = NULL;
	}

	g_debug("eas_email_info_serialise--");
	return ret;
}

gboolean 
eas_email_info_deserialise(EasEmailInfo* self, const gchar *data)
{
	// TODO proper error handling - eg deal with get_next_field returning null
	gboolean ret = TRUE;
	guint list_len = 0, i = 0;
	gchar *list_len_as_string = NULL;
	EasEmailHeader *header = NULL;
	EasAttachment *attachment = NULL;
	GSList *headers = NULL;
	GSList *attachments = NULL;
	GSList *categories = NULL;
	gchar *from = (gchar*)data;
    gchar *flags_as_string = NULL;
    
    g_debug("eas_email_info_deserialise++");
	g_assert(self);
	g_assert(data);
	
	// turn string into object
	// server_id
	if(self->server_id != NULL)   //just in case
	{
		g_free(self->server_id);
	}
	self->server_id = get_next_field(&from, sep);	
	if(!self->server_id)
	{
		ret = FALSE;
		goto cleanup;
	}
	g_debug("server_id = %s", self->server_id);

	//headers
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
	g_debug("%d headers", list_len);

	for(i = 0; i < list_len; i++)
	{
		header = g_malloc0(sizeof(EasEmailHeader));
		header->name = get_next_field(&from, sep);
		if(!header->name)
		{
			ret = FALSE;
			goto cleanup;
		}
		header->value = get_next_field(&from, sep);
		if(!header->value)
		{
			ret = FALSE;
			goto cleanup;
		}
		headers = g_slist_append(headers, header);
	}
	self->headers = headers;
	
	//attachments
	list_len_as_string = get_next_field(&from, sep);
	if(!list_len_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
	g_debug("%d attachments", list_len);
	for(i = 0; i < list_len; i++)
	{
		attachment = eas_attachment_new ();
		if(!eas_attachment_deserialise(attachment, from))
		{
			ret = FALSE;
			goto cleanup;
		}
		from += eas_attachment_serialised_length (attachment);//attachment deserialise doesn't move pointer along

		attachments = g_slist_append(attachments, attachment);
	}
	self->attachments = attachments;
	
	//flags
	flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		self->flags = atoi(flags_as_string);
	}
	g_free(flags_as_string);
	flags_as_string = NULL;
	g_debug("flags = %d", self->flags);	
	
	//categories
	list_len_as_string = get_next_field(&from, sep);
	list_len = atoi(list_len_as_string);
	g_free(list_len_as_string);
	list_len_as_string = NULL;
	g_debug("%d categories", list_len);	

	for(i = 0; i < list_len; i++)
	{
		gchar *category = get_next_field(&from, sep);
		if(!category)
		{
			ret = FALSE;
			goto cleanup;
		}
		categories = g_slist_append(categories, category);
	}	
	self->categories = categories;

	//estimated_size
	flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		self->estimated_size = strtoul(flags_as_string, NULL, 10);
	}
	g_free(flags_as_string);
	flags_as_string = NULL;
	g_debug("estimated size = %zu", self->estimated_size);

	//date_received
	flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		self->date_received = strtoul(flags_as_string, NULL, 10);
	}
	g_free(flags_as_string);
	flags_as_string = NULL;
	g_debug("date received = %ld", self->date_received);

	//date_received
	flags_as_string = get_next_field(&from, sep);
	if(!flags_as_string)
	{
		ret = FALSE;
		goto cleanup;
	}
	if(strlen(flags_as_string))
	{
		self->importance = strtoul(flags_as_string, NULL, 10);
	}
	g_free(flags_as_string);
	flags_as_string = NULL;
	g_debug("importance = %d", self->importance);

cleanup:
	
	if(!ret)
	{
		g_debug("failed!");
		//TODO cleanup
	}

	g_debug("eas_email_info_deserialise--");
	return ret;
}

