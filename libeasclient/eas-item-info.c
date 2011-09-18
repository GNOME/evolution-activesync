/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ActiveSync client library for calendar/addressbook synchronisation
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

#include "eas-item-info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const gchar SERVER_ID_SEPARATOR = '\n';


G_DEFINE_TYPE (EasItemInfo, eas_item_info, G_TYPE_OBJECT);


static void eas_item_info_init (EasItemInfo *object)
{
	g_debug ("eas_item_info_init++");

	object->client_id = NULL;
	object->server_id = NULL;
	object->data = NULL;
	object->status = NULL;
}


static void eas_item_info_finalize (GObject* object)
{
	EasItemInfo* self = EAS_ITEM_INFO (object);

	g_free (self->client_id);
	g_free (self->server_id);
	g_free (self->data);
	g_free (self->status);

	G_OBJECT_CLASS (eas_item_info_parent_class)->finalize (object);
}


static void eas_item_info_class_init (EasItemInfoClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_item_info_finalize;
}


EasItemInfo* eas_item_info_new ()
{
	EasItemInfo *object = g_object_new (EAS_TYPE_ITEM_INFO , NULL);
	g_debug ("eas_item_info_new+-");
	return object;
}


gboolean eas_item_info_serialise (EasItemInfo* self, gchar** result)
{
	GString* str = NULL;



	str = g_string_new ( (self->client_id ? : ""));
	str = g_string_append_c (str, SERVER_ID_SEPARATOR);
	str = g_string_append (str, (self->server_id ? : ""));
	str = g_string_append_c (str, SERVER_ID_SEPARATOR);
	str = g_string_append (str, (self->status ? : ""));
	str = g_string_append_c (str, SERVER_ID_SEPARATOR);
	str = g_string_append (str, (self->data ? : ""));
	*result = g_string_free (str, FALSE); // Destroy the GString but not the buffer (which is returned with ownership)
	return TRUE;
}


gboolean eas_item_info_deserialise (EasItemInfo* self, const gchar* data)
{
	gboolean separator_found = FALSE;
	guint i = 0;
	gchar *tempString = NULL;
	gchar *tempString2 = NULL;


	g_debug ("eas_item_info_deserialise++");

	//check that there is data to deserialise - otherwise return false
	if (data == NULL || strlen (data) == 0) {
		return FALSE;
	}
	// Look for the separator character
	for (; data[i]; i++) {
		if (data[i] == SERVER_ID_SEPARATOR) {
			separator_found = TRUE;
			break;
		}
	}

	if (separator_found) {
		self->client_id = g_strndup (data, i);
		tempString = g_strdup (data + (i + 1));
		separator_found = FALSE;
	}
	i = 0;

	for (; tempString[i]; i++) {
		if (tempString[i] == SERVER_ID_SEPARATOR) {
			separator_found = TRUE;
			break;
		}
	}
	if (separator_found) {
		self->server_id = g_strndup (tempString, i);
		tempString2 = g_strdup (tempString + (i + 1));
		separator_found = FALSE;
	}
	i = 0;
	for (; tempString2[i]; i++) {
		if (tempString2[i] == SERVER_ID_SEPARATOR) {
			separator_found = TRUE;
			break;
		}
	}

	if (separator_found) {
		self->status = g_strndup (tempString2, i);
		self->data = g_strdup (tempString2 + (i + 1));
	}

	g_free (tempString);
	g_free (tempString2);

	return separator_found;
}

