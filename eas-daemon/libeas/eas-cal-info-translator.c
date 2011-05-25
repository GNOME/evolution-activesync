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


/**
 * \brief Private function to concatenate a VCALENDAR property name and value,
 *        terminate with a newline and append the whole thing to a list
 *
 * \param list              The list to append to
 * \param prop_name         The property name
 * \param prop_value_node   The XML node providing the property value
 */
static void _util_append_prop_string_to_list(GSList** list, const gchar* prop_name, const gchar* prop_value)
{
	gchar* property = g_strconcat(prop_name, ":", prop_value, "\n", NULL);
	*list = g_slist_append(*list, property); // Takes ownership of the property string
}


/**
 * \brief Private function to concatenate a VCALENDAR property name and value (latter taken from an XML node),
 *        terminate with a newline and append the whole thing to a list
 *
 * \param list              The list to append to
 * \param prop_name         The property name
 * \param prop_value_node   The XML node providing the property value
 */
static void _util_append_prop_string_to_list_from_xml(GSList** list, const gchar* prop_name, xmlNode* prop_value_node)
{
	_util_append_prop_string_to_list (list, prop_name, (const gchar*)prop_value_node->content);
}


// Parse a response message
gchar* eas_cal_info_translator_parse_response(EasCalInfoTranslator* self, xmlNode* node, gchar* server_id)
{
	// TODO: does this even have to be a class? We don't seem to be using self at all!
	
	gchar* result = NULL;

	if (node && (node->type == XML_ELEMENT_NODE) && (!strcmp((char*)(node->name), "ApplicationData")))
	{
//		EasCalInfo* cal_info = eas_cal_info_new();
		xmlNode* n = node;

		guint bufferSize = 0;

		// Using separate lists to capture the data for various parts of the iCalendar document:
		//
		//   BEGIN VCALENDAR
		//      [ BEGIN VTIMEZONE ... END VTIMEZONE ]
		//      BEGIN VEVENT
		//         [ BEGIN VALARM ... END VALARM ]
		//      END VEVENT
		//   END VCALENDAR
		GSList* vcalendar = NULL;
		GSList* vtimezone = NULL;
		GSList* vevent = NULL;
		GSList* valarm = NULL;

		// TODO: get all these strings into constants/#defines
		
		// TODO: make the PRODID configurable somehow
		_util_append_prop_string_to_list(&vcalendar, "PRODID", "-//Meego//ActiveSyncD 1.0//EN");
		_util_append_prop_string_to_list(&vcalendar, "VERSION", "2.0");
		_util_append_prop_string_to_list(&vcalendar, "METHOD", "PUBLISH");
		
		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);

				if (g_strcmp0(name, "Subject") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "SUMMARY", n);
				}
				else if (g_strcmp0(name, "StartTime") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "DTSTART", n);
				}
				else if (g_strcmp0(name, "EndTime") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "DTEND", n);
				}
				else if (g_strcmp0(name, "DtStamp") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "DTSTAMP", n);
				}
				else if (g_strcmp0(name, "UID") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "UID", n);
				}
				else if (g_strcmp0(name, "Location") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "LOCATION", n);
				}
				else if (g_strcmp0(name, "Body") == 0)
				{
					_util_append_prop_string_to_list_from_xml(&vevent, "DESCRIPTION", n);
				}
				else if (g_strcmp0(name, "Sensitivity") == 0)
				{
					const gchar* value = (const gchar*)n->content;
					if (g_strcmp0(value, "3") == 0)      // Confidential
					{
						_util_append_prop_string_to_list(&vevent, "CLASS", "CONFIDENTIAL");
					}
					else if (g_strcmp0(value, "2") == 0) // Private
					{
						_util_append_prop_string_to_list(&vevent, "CLASS", "PRIVATE");
					}
					else // Personal or Normal (iCal doesn't distinguish between them)
					{
						_util_append_prop_string_to_list(&vevent, "CLASS", "PUBLIC");
					}
				}
				else if (g_strcmp0(name, "BusyStatus") == 0)
				{
					const gchar* value = (const gchar*)n->content;
					if (g_strcmp0(value, "0") == 0) // Free
					{
						_util_append_prop_string_to_list(&vevent, "TRANSP", "TRANSPARENT");
					}
					else // Tentative, Busy or Out of Office
					{
						_util_append_prop_string_to_list(&vevent, "TRANSP", "OPAQUE");
					}
				}
				else if (g_strcmp0(name, "Categories") == 0)
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
						_util_append_prop_string_to_list(&vevent, "CATEGORIES", categories->str);
					}

					// Free the string, including the character buffer
					g_string_free(categories, TRUE);
				}
				else if (g_strcmp0(name, "Reminder") == 0)
				{
					gchar* trigger = g_strconcat("-P", n->content, "M", NULL);
					_util_append_prop_string_to_list(&valarm, "ACTION", "DISPLAY");
					_util_append_prop_string_to_list(&valarm, "DESCRIPTION", "Reminder");   // TODO: make this configurable
					_util_append_prop_string_to_list(&valarm, "TRIGGER", trigger);
					g_free(trigger);
				}

				// TODO: handle Timezone element
				// TODO: handle Attendees element
				// TODO: handle Recurrence element
				// TODO: handle Exceptions element
			}
		}

		// TODO: Think of a way to pre-allocate a sensible size for the buffer
		GString* vcal_buf = g_string_new("BEGIN:VCALENDAR\n");

		// Add the VCALENDAR properties first
		GSList* item;
		for (item = vcalendar; item != NULL; item = item->next)
		{
			// Add a string per list item then destroy the list item behind us
			vcal_buf = g_string_append(vcal_buf, (const gchar*)item->data);
			g_free(item->data);
		}
		
		// Now add the timezone (if there is one)
		if ((item = vtimezone) != NULL)
		{
			vcal_buf = g_string_append(vcal_buf, "BEGIN:VTIMEZONE\n");
			for (; item != NULL; item = item->next)
			{
				// Add a string per list item then destroy the list item behind us
				vcal_buf = g_string_append(vcal_buf, (const gchar*)item->data);
				g_free(item->data);
			}
			vcal_buf = g_string_append(vcal_buf, "END:VTIMEZONE\n");
		}

		// Now add the event
		if ((item = vevent) != NULL)
		{
			vcal_buf = g_string_append(vcal_buf, "BEGIN:VEVENT\n");
			for (; item != NULL; item = item->next)
			{
				// Add a string per list item then destroy the list item behind us
				vcal_buf = g_string_append(vcal_buf, (const gchar*)item->data);
				g_free(item->data);
			}

			// Now add the alarm (nested inside the event)
			if ((item = valarm) != NULL)
			{
				vcal_buf = g_string_append(vcal_buf, "BEGIN:VALARM\n");
				for (; item != NULL; item = item->next)
				{
					// Add a string per list item then destroy the list item behind us
					vcal_buf = g_string_append(vcal_buf, (const gchar*)item->data);
					g_free(item->data);
				}
				vcal_buf = g_string_append(vcal_buf, "END:VALARM\n");
			}

			vcal_buf = g_string_append(vcal_buf, "END:VEVENT\n");
		}

		// Delete the lists
		g_slist_free(vcalendar);
		g_slist_free(vtimezone);
		g_slist_free(vevent);
		g_slist_free(valarm);
		                                 
		// And finally close the VCALENDAR
		vcal_buf = g_string_append(vcal_buf, "END:VCALENDAR\n");

		result = g_string_free(vcal_buf, FALSE); // Frees the GString object and returns its buffer with ownership
	}

	return result;
}
