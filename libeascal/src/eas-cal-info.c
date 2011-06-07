/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 */


#include "eas-cal-info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const gchar SERVER_ID_SEPARATOR = '\n';


G_DEFINE_TYPE (EasCalInfo, eas_cal_info, G_TYPE_OBJECT);


static void eas_cal_info_init (EasCalInfo *object)
{
	g_debug("eas_cal_info_init++");

	object->client_id = NULL;
	object->server_id = NULL;
	object->icalendar = NULL;
}


static void eas_cal_info_finalize (GObject* object)
{
	EasCalInfo* self = (EasCalInfo*)object;

	g_free(self->client_id);
	g_free(self->server_id);
	g_free(self->icalendar);

	G_OBJECT_CLASS (eas_cal_info_parent_class)->finalize (object);
}


static void eas_cal_info_class_init (EasCalInfoClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	// TODO better way to get rid of warnings about above 2 lines?
	void *temp = (void*)object_class;
	temp = (void*)parent_class;

	object_class->finalize = eas_cal_info_finalize;
}


EasCalInfo* eas_cal_info_new ()
{
	g_debug("eas_cal_info_new++");	
	
	EasCalInfo *object = NULL;
	object = g_object_new (EAS_TYPE_CAL_INFO , NULL);
	g_debug("eas_cal_info_new--");	
	
	return object;
}


gboolean eas_cal_info_serialise(EasCalInfo* self, gchar** result)
{
	GString* str = g_string_new(self->client_id);
	str = g_string_append_c(str, SERVER_ID_SEPARATOR);
	str = g_string_append(str, self->server_id);
	str = g_string_append_c(str, SERVER_ID_SEPARATOR);
	str = g_string_append(str, self->icalendar);
	*result = g_string_free(str, FALSE); // Destroy the GString but not the buffer (which is returned with ownership)
	return TRUE;
}


gboolean eas_cal_info_deserialise(EasCalInfo* self, const gchar* data)
{
	gboolean separator_found = FALSE;
	g_debug("eas_cal_info_deserialise++");
	gchar *tempString = NULL;
	// Look for the separator character
	guint i = 0;
	for (; data[i]; i++)
	{
		if (data[i] == SERVER_ID_SEPARATOR)
		{
			separator_found = TRUE;
			break;
		}
	}

	if (separator_found)
	{
		self->client_id = g_strndup(data, i);
		tempString = g_strdup(data + (i + 1));
		separator_found = FALSE;
	}
	i = 0;
	for (; tempString[i]; i++)
	{
		if (tempString[i] == SERVER_ID_SEPARATOR)
		{
			separator_found = TRUE;
			break;
		}
	}

	if (separator_found)
	{
		self->server_id = g_strndup(tempString, i);
		self->icalendar = g_strdup(data + (i + 1));
	}		
	
	return separator_found;
}

