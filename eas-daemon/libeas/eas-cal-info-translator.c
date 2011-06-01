/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */


#include "eas-cal-info-translator.h"

#include <icalparser.h>
#include <icalcomponent.h>
#include <icaltypes.h>
#include <icalduration.h>

#include <libwbxml-1.0/wbxml/wbxml.h>

// iCalendar constants defined in RFC5545 (http://tools.ietf.org/html/rfc5545)
const gchar* ICAL_LINE_TERMINATOR           = "\r\n";
const guint  ICAL_LINE_TERMINATOR_LENGTH    = 2;
const gchar* ICAL_FOLDING_SEPARATOR         = "\r\n ";   // ICAL_LINE_TERMINATOR followed by single whitespace character (space or tab)
const guint	 ICAL_MAX_LINE_LENGTH           = 75;

// Values for converting icaldurationtype into a number of minutes
const guint MINUTES_PER_HOUR = 60;
const guint MINUTES_PER_DAY  = 60 * 24;
const guint MINUTES_PER_WEEK = 60 * 24 * 7;

typedef  struct {
	gushort wYear;
	gushort wMonth;
	gushort wDayOfWeek;
	gushort wDay;
	gushort wHour;
	gushort wMinute;
	gushort wSecond;
	gushort wMilliseconds;
} EasSystemTime;

typedef struct {
	glong Bias;
	wchar_t StandardName[32];
	EasSystemTime StandardDate;
	glong StandardBias;
	wchar_t DaylightName[32];
	EasSystemTime DaylightDate;
	glong DaylightBias;
} EasTimeZone;
	

/**
 * \brief Private function to concatenate a VCALENDAR property name and value
 *        and append it to a list
 *
 * \param list              The list to append to
 * \param prop_name         The property name
 * \param prop_value_node   The XML node providing the property value
 */
static void _util_append_prop_string_to_list(GSList** list, const gchar* prop_name, const gchar* prop_value)
{
	gchar* property = g_strconcat(prop_name, ":", prop_value, NULL);
	*list = g_slist_append(*list, property); // Takes ownership of the property string
}


/**
 * \brief Private function to concatenate a VCALENDAR property name and value (latter taken from an XML node),
 *        and append it to a list.
 *
 * \param list              The list to append to
 * \param prop_name         The property name
 * \param prop_value_node   The XML node providing the property value
 */
static void _util_append_prop_string_to_list_from_xml(GSList** list, const gchar* prop_name, xmlNode* prop_value_node)
{
	xmlChar* prop_value = xmlNodeGetContent(prop_value_node);
	_util_append_prop_string_to_list (list, prop_name, (const gchar*)prop_value);
	xmlFree(prop_value);
}


/**
 * \brief Private function to append a line to a iCalendar-format buffer. Replaces newlines with a \n escape
 *        sequences and also "folds" long line into shorter lines confirming with the maximum line length
 *        specified in RFC-5545.
 *
 * \param buffer               The iCalendar buffer to append the line to
 * \param line                 The line to append (must be null-terminated)
 * \param may_contan_newlines  TRUE if the line should be scanned for newline sequences (requires constructing
 *                             a local GRegex object) or FALSE if it's guaranteed to be a single line string
 *                             (e.g. a literal such as "BEGIN:VCALENDAR") in which case the GRegex overhead
 *                             can be avoided.
 */
static void _util_append_line_to_ical_buffer(GString** buffer, const gchar* line, gboolean may_contain_newlines)
{
	// TODO: there's a lot of string copying going on in this function.
	// Need to think if thee's a more efficient way of doing this.

	
	// Start by replacing any newlines in the line with a \n character sequence
	gchar* line_with_newlines_handled = NULL;
	if (may_contain_newlines)
	{
		GError* error = NULL;
		GRegex* regex_newline = g_regex_new("\\R", 0, 0, &error); // \R in a regex matches any kind of newline (\r\n, \r or \n)
		line_with_newlines_handled = g_regex_replace_literal(regex_newline, line, -1, 0, "\\n", 0, &error);
		g_regex_unref(regex_newline); // Destroy the regex
		if (error)
		{
			g_error_free(error); // Destroy the error
		}
	}
	else
	{
		line_with_newlines_handled = (char*)line;
	}
	
	// Now fold the string into max 75-char lines
	// (See http://tools.ietf.org/html/rfc5545#section-3.1)
	
	GString* line_for_folding = g_string_new(line_with_newlines_handled);
	// Use an index to mark the position the next line should be started at
	guint next_line_start = ICAL_MAX_LINE_LENGTH;
	while (next_line_start < line_for_folding->len)
	{
		// Insert a folding break (line break followed by whitespace)
		line_for_folding = g_string_insert(line_for_folding, next_line_start, ICAL_FOLDING_SEPARATOR);
		// And now update the index to find the next new line start position
		next_line_start += (ICAL_LINE_TERMINATOR_LENGTH + ICAL_MAX_LINE_LENGTH);
	}

	*buffer = g_string_append(*buffer, line_for_folding->str);
	*buffer = g_string_append(*buffer, ICAL_LINE_TERMINATOR);

	g_string_free(line_for_folding, TRUE); // Destroy the GString and its buffer

	if (may_contain_newlines)
	{
		g_free(line_with_newlines_handled);
	}
}


/**
 * \brief Parse an XML-formatted calendar object received from ActiveSync and return
 *        it as a serialised iCalendar object.
 *
 * \param node       ActiveSync XML <ApplicationData> object containing a calendar.
 * \param server_id  The ActiveSync server ID from the response
 */
gchar* eas_cal_info_translator_parse_response(xmlNodePtr node, const gchar* server_id)
{
	// TODO: Oops! I only found libical after I'd implemented this.
	// We should switch to libical - it will make further development a lot easier and more robust.

	
	gchar* result = NULL;

	if (node && (node->type == XML_ELEMENT_NODE) && (!strcmp((char*)(node->name), "ApplicationData")))
	{
		xmlNodePtr n = node;

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
					xmlNode *subNode = NULL;
					for (subNode = n->children; subNode; subNode = subNode->next)
					{
						if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0(subNode->name, "Data"))
						{
							_util_append_prop_string_to_list_from_xml(&vevent, "DESCRIPTION", subNode);
							break;
						}
					}
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
					GString* categories = g_string_new("");

					xmlNode* catNode = NULL;
					for (catNode = n->children; catNode; catNode = catNode->next)
					{
						if (categories->len > 0)
						{
							categories = g_string_append(categories, ",");
						}

						xmlChar* category = xmlNodeGetContent(catNode);
						categories = g_string_append(categories, (const gchar*)category);
						xmlFree(category);
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
					xmlChar* minutes = xmlNodeGetContent(n);
					gchar* trigger = g_strconcat("-P", (const char*)minutes, "M", NULL);
					_util_append_prop_string_to_list(&valarm, "ACTION", "DISPLAY");
					_util_append_prop_string_to_list(&valarm, "DESCRIPTION", "Reminder");   // TODO: make this configurable
					_util_append_prop_string_to_list(&valarm, "TRIGGER", trigger);
					g_free(trigger);
					xmlFree(minutes);
				}
				else if (g_strcmp0(name, "TimeZone") == 0)
				{
					xmlChar *timeZoneB64 = xmlNodeGetContent(n);
					EasTimeZone tz;
					WB_UTINY *timeZone = NULL;
					
					g_debug("TimeZoneB64 [%s], %d", timeZoneB64, strlen(timeZoneB64));

					wbxml_base64_decode(timeZoneB64, -1, &timeZone);
					xmlFree(timeZoneB64);
					memcpy(&tz, timeZone, sizeof(tz));

					// TODO: Copy the TimeZone Data from tz into the vcal
					
					g_free(timeZone);
				}
				
				// TODO: handle Attendees element
				// TODO: handle Recurrence element
				// TODO: handle Exceptions element
			}
		}

		// TODO: Think of a way to pre-allocate a sensible size for the buffer
		GString* ical_buf = g_string_new("");
		_util_append_line_to_ical_buffer(&ical_buf, "BEGIN:VCALENDAR", FALSE);

		// Add the VCALENDAR properties first
		GSList* item;
		for (item = vcalendar; item != NULL; item = item->next)
		{
			// Add a string per list item then destroy the list item behind us
			_util_append_line_to_ical_buffer(&ical_buf, (const gchar*)item->data, TRUE);
			g_free(item->data);
		}
		
		// Now add the timezone (if there is one)
		if ((item = vtimezone) != NULL)
		{
			_util_append_line_to_ical_buffer(&ical_buf, "BEGIN:VTIMEZONE", FALSE);
			for (; item != NULL; item = item->next)
			{
				// Add a string per list item then destroy the list item behind us
				_util_append_line_to_ical_buffer(&ical_buf, (const gchar*)item->data, TRUE);
				g_free(item->data);
			}
			_util_append_line_to_ical_buffer(&ical_buf, "END:VTIMEZONE", FALSE);
		}

		// Now add the event
		if ((item = vevent) != NULL)
		{
			_util_append_line_to_ical_buffer(&ical_buf, "BEGIN:VEVENT", FALSE);
			for (; item != NULL; item = item->next)
			{
				// Add a string per list item then destroy the list item behind us
				_util_append_line_to_ical_buffer(&ical_buf, (const gchar*)item->data, TRUE);
				g_free(item->data);
			}

			// Now add the alarm (nested inside the event)
			if ((item = valarm) != NULL)
			{
				_util_append_line_to_ical_buffer(&ical_buf, "BEGIN:VALARM", FALSE);
				for (; item != NULL; item = item->next)
				{
					// Add a string per list item then destroy the list item behind us
					_util_append_line_to_ical_buffer(&ical_buf, (const gchar*)item->data, TRUE);
					g_free(item->data);
				}
				_util_append_line_to_ical_buffer(&ical_buf, "END:VALARM", FALSE);
			}

			_util_append_line_to_ical_buffer(&ical_buf, "END:VEVENT", FALSE);
		}

		// Delete the lists
		g_slist_free(vcalendar);
		g_slist_free(vtimezone);
		g_slist_free(vevent);
		g_slist_free(valarm);
		                                 
		// And finally close the VCALENDAR
		_util_append_line_to_ical_buffer(&ical_buf, "END:VCALENDAR", FALSE);

		// Now insert the server ID and iCalendar into an EasCalInfo object and serialise it
		EasCalInfo* cal_info = eas_cal_info_new();
		cal_info->icalendar = g_string_free(ical_buf, FALSE); // Destroy the GString object and pass its buffer (with ownership) to cal_info
		cal_info->server_id = (gchar*)server_id;
		if (!eas_cal_info_serialise(cal_info, &result))
		{
			// TODO: log error
			result = NULL;
		}

		g_object_unref(cal_info);
	}

	return result;
}


static void _util_process_vevent_component(icalcomponent* vevent, xmlNodePtr app_data)
{
	if (vevent)
	{
		xmlNodePtr categories = NULL;
		
		icalproperty* prop;
		for (prop = icalcomponent_get_first_property(vevent, ICAL_ANY_PROPERTY);
			 prop;
			 prop = icalcomponent_get_next_property(vevent, ICAL_ANY_PROPERTY))
		{
			const icalproperty_kind prop_type = icalproperty_isa(prop);
			switch (prop_type)
			{
				case ICAL_SUMMARY_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:Subject", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTSTAMP_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:DtStamp", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTSTART_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:StartTime", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTEND_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:EndTime", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_LOCATION_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:Location", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_UID_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:UID", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_CLASS_PROPERTY:
					{
						const gchar* value = (const gchar*)icalproperty_get_value_as_string(prop);
						if (g_strcmp0(value, "CONFIDENTIAL") == 0)
						{
							xmlNewTextChild(app_data, NULL, "calendar:Sensitivity", "3"); // Confidential
						}
						else if (g_strcmp0(value, "PRIVATE") == 0)
						{
							xmlNewTextChild(app_data, NULL, "calendar:Sensitivity", "2"); // Private
						}
						else // PUBLIC
						{
							// iCalendar doesn't distinguish between 0 (Normal) and 1 (Personal)
							xmlNewTextChild(app_data, NULL, "calendar:Sensitivity", "0");
						}
					}
					break;
				case ICAL_TRANSP_PROPERTY:
					{
						const gchar* value = (const gchar*)icalproperty_get_value_as_string(prop);
						if (g_strcmp0(value, "TRANSPARENT") == 0)
						{
							xmlNewTextChild(app_data, NULL, "calendar:BusyStatus", "0"); // Free
						}
						else // OPAQUE
						{
							// iCalendar doesn't distinguish between 1 (Tentative), 2 (Busy), 3 (Out of Office)
							xmlNewTextChild(app_data, NULL, "calendar:BusyStatus", "2"); // Busy
						}
					}
					break;
				case ICAL_CATEGORIES_PROPERTY:
					{
						if (categories == NULL)
						{
							categories = xmlNewChild(app_data, NULL, "calendar:Categories", NULL);
						}

						xmlNewTextChild(categories, NULL, "calendar:Category", icalproperty_get_value_as_string(prop));
					}
					break;

/*				case ICAL_xxx_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:xxx", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_xxx_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:xxx", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_xxx_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:xxx", icalproperty_get_value_as_string(prop));
					break;*/
				// TODO: all the rest :)

				default:
					break;
			}
		}
	}
}


static void _util_process_valarm_component(icalcomponent* valarm, xmlNodePtr app_data)
{
	if (valarm)
	{
		g_debug("Processing VALARM...");
		
		// Just need to get the TRIGGER property
		icalproperty* prop = icalcomponent_get_first_property(valarm, ICAL_TRIGGER_PROPERTY);
		if (prop)
		{
			g_debug("Processing TRIGGER...");
			
			struct icaltriggertype trigger = icalproperty_get_trigger(prop);

			// TRIGGER can be either a period of time before the event, OR a specific date/time.
			// calendar:Reminder only accepts a number of minutes

			// For now I'm ASSUMING it'll be the former.
			// TODO: handle the latter as well

			// As ActiveSync only accepts minutes we'll ignore the seconds value altogether
			guint minutes = trigger.duration.minutes
				+ (trigger.duration.hours * MINUTES_PER_HOUR)
				+ (trigger.duration.days  * MINUTES_PER_DAY)
				+ (trigger.duration.weeks * MINUTES_PER_WEEK);

			char minutes_buf[6];
			g_snprintf(minutes_buf, 6, "%d", minutes);

			xmlNewTextChild(app_data, NULL, "calendar:Reminder", minutes_buf);
		}
	}
}


static void _util_process_vtimezone_component(icalcomponent* vtimezone, xmlNodePtr app_data)
{
	if (vtimezone)
	{
		icalproperty* prop;
		for (prop = icalcomponent_get_first_property(vtimezone, ICAL_ANY_PROPERTY);
			 prop;
			 prop = icalcomponent_get_next_property(vtimezone, ICAL_ANY_PROPERTY))
		{
			const icalproperty_kind prop_type = icalproperty_isa(prop);
			switch (prop_type)
			{
/*				case ICAL_xxx_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:xxx", icalproperty_get_value_as_string(prop));
					break;
				case ICAL_xxx_PROPERTY:
					xmlNewTextChild(app_data, NULL, "calendar:xxx", icalproperty_get_value_as_string(prop));
					break;*/
				// TODO: all the rest :)

				default:
					break;
			}
		}
	}
}


/**
 *
 */
gboolean eas_cal_info_translator_parse_request(xmlDocPtr doc, xmlNodePtr app_data, EasCalInfo* cal_info)
{
	gboolean success = FALSE;

	icalcomponent* ical;
	if (doc &&
	    app_data &&
	    cal_info &&
	    (app_data->type == XML_ELEMENT_NODE) &&
	    (strcmp((char*)(app_data->name), "ApplicationData") == 0) &&
	    (ical = icalparser_parse_string(cal_info->icalendar)) &&
	    (icalcomponent_isa(ical) == ICAL_VCALENDAR_COMPONENT))
	{
		// Process the components of the VCALENDAR
		icalcomponent* vevent = icalcomponent_get_first_component(ical, ICAL_VEVENT_COMPONENT);
		_util_process_vevent_component(vevent, app_data);
		_util_process_valarm_component(icalcomponent_get_first_component(vevent, ICAL_VALARM_COMPONENT), app_data);
		_util_process_vtimezone_component(icalcomponent_get_first_component(ical, ICAL_VTIMEZONE_COMPONENT), app_data);

		// DEBUG output
		xmlChar* dump_buffer;
		int dump_buffer_size;
		xmlIndentTreeOutput = 1;
		xmlDocDumpFormatMemory(doc, &dump_buffer, &dump_buffer_size, 1);
		g_debug("XML DOCUMENT DUMPED:\n%s", dump_buffer);
		xmlFree(dump_buffer);

		success = TRUE;
	}

	if (ical)
	{
		icalcomponent_free(ical);
	}

	return success;
}

