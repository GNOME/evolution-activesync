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

		// Using separate lits to capture the data for various parts of the iCalendar document:
		//
		//   BEGIN VCALENDAR
		//      [ BEGIN VTMEZONE ... END VTIMEZONE ]
		//      BEGIN VEVENT
		//         [ BEGIN VALARM ... END VALARM ]
		//      END VEVENT
		//   END VCALENDAR
		GSList* vcalendar = NULL;

		GSList* vevent = NULL;
		GSList* valarm = NULL;
		GSList* vtimezone = NULL;

		// TODO: get all these strings into constants/#defines
		
		vcalendar = g_slist_append(vcalendar, g_strdup("BEGIN:VCALENDAR"));
		// TODO: make the PRODID configurable somehow
		vcalendar = g_slist_append(vcalendar, g_strdup("PRODID:-//Meego//ActiveSyncD 1.0//EN"));
		vcalendar = g_slist_append(vcalendar, g_strdup("VERSION:2.0"));
		vcalendar = g_slist_append(vcalendar, g_strdup("METHOD:PUBLISH"));
		
		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);

				if (g_strcmp0(name, "calendar:Subject") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("SUMMARY:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:StartTime") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("DTSTART:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:EndTime") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("DTEND:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:DtStamp") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("DTSTAMP:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:UID") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("UID:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:Location") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("LOCATION:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "airsyncbase:Body") == 0)
				{
					vevent = g_slist_append(vevent, g_strconcat("DESCRIPTION:", (const gchar*)n->content));
				}
				else if (g_strcmp0(name, "calendar:Categories") == 0)
				{
					GString* categories = NULL;

					xmlNode* catNode = NULL;
					for (catNode = n->children; catNode; catNode = catNode->next)
					{
						if (categories->len == 0)
						{
							categories = g_string_new((const gchar*)catNode->content);
						}
						else
						{
							categories = g_string_append(categories, ",");
							categories = g_string_append(categories, (const gchar*)catNode->content);
						}
					}
					if (categories->len > 0)
					{
						vevent = g_slist_append(vevent, g_strconcat("CATEGORIES:", categories->str));
					}

					// Free the string, including the character buffer
					g_string_free(categories, TRUE);
				}
			}
		}

   		vcalendar = g_slist_append(vcalendar, g_strdup("END:VEVENT"));

	}

	return result;
}
