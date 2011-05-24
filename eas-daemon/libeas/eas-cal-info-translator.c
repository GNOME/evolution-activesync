/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */

#include "eas-cal-info-translator.h"

#include "../../libeascal/src/eas-cal-info.h"



G_DEFINE_TYPE (EasCalInfoTranslator, eas_cal_info_translator, G_TYPE_OBJECT);

static void eas_cal_info_translator_init (EasCalInfoTranslator *object)
{
}


static void eas_cal_info_translator_finalize (GObject *object)
{
	G_OBJECT_CLASS (eas_cal_info_translator_parent_class)->finalize (object);
}


static void eas_cal_info_translator_class_init (EasCalInfoTranslatorClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_cal_info_translator_finalize;
}


// Constructor
EasCalInfoTranslator* eas_cal_info_translator_new()
{
}


// Parse a response message
gchar* eas_cal_info_translator_parse_response(EasCalInfoTranslator* self, xmlNode* node, gchar* server_id)
{
	// TODO: does this even have to be a class? We don't seem to be using self at all!
	
	gchar* result = NULL;

	if (node && (node->type == XML_ELEMENT_NODE) && (!strcmp((char*)(node->name), "ApplicationData")))
	{
		EasCalInfo* cal_info = eas_cal_info_new();
		xmlNode* n = node;

		GSList* iCalLines = NULL;

		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const char* name = (const char*)(node->name);

				if (strcmp(name, "calendar:DtStamp"))
				{
					// TODO: implement
				}
				else if (strcmp(name, "calendar:Subject"))
				{
					// TODO: implement
				}
				// TODO: continue
			}
		}
	}

	return result;
}
