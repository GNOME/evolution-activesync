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



G_DEFINE_TYPE (EasCalInfo, eas_cal_info, G_TYPE_OBJECT);


static void eas_cal_info_init (EasCalInfo *object)
{
	g_debug("eas_cal_info_init++");

	object->server_id = NULL;
	object->icalendar = NULL;
}


static void eas_cal_info_finalize (GObject* object)
{
	EasCalInfo* self = (EasCalInfo*)object;

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
	// Temporary
	// TODO: complete
	return FALSE;
}


gboolean eas_email_info_deserialise(EasCalInfo* self, const gchar* data)
{
	// Temporary
	// TODO: complete
	return FALSE;
}