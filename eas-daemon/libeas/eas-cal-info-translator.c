/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * git
 * Copyright (C)  2011 <>
 * 
 */


#include "eas-cal-info-translator.h"

#include <libical/icalparser.h>
#include <libical/icalcomponent.h>
#include <libical/icaltypes.h>
#include <libical/icalduration.h>

#include <libwbxml-1.0/wbxml/wbxml.h>

// iCalendar constants defined in RFC5545 (http://tools.ietf.org/html/rfc5545)
const gchar* ICAL_LINE_TERMINATOR           = "\r\n";
const guint  ICAL_LINE_TERMINATOR_LENGTH    = 2;
const gchar* ICAL_FOLDING_SEPARATOR         = "\r\n ";   // ICAL_LINE_TERMINATOR followed by single whitespace character (space or tab)
const guint	 ICAL_MAX_LINE_LENGTH           = 75;

// Values for converting icaldurationtype into a number of minutes
const gint SECONDS_PER_MINUTE = 60;
const gint MINUTES_PER_HOUR = 60;
const gint MINUTES_PER_DAY  = 60 * 24;
const gint MINUTES_PER_WEEK = 60 * 24 * 7;

#define compile_time_assert(cond, msg) \
	char msg[(cond)?1:-1]

typedef  struct {
	guint16 Year;
	guint16 Month;
	guint16 DayOfWeek;
	guint16 Day;
	guint16 Hour;
	guint16 Minute;
	guint16 Second;
	guint16 Millisecond;
} __attribute__((packed)) EasSystemTime;

compile_time_assert((sizeof(EasSystemTime) == 16), EasSystemTime_not_expected_size);

/* From ActiveSync Protocol Doc MS-ASDTYPE
 * The required values are Bias, which is the offset from UTC, in minutes, and 
 * the StandardDate and DaylightDate, which are needed when the biases take 
 * effect. For example, the bias for Pacific Time is 480.
 */
typedef struct {
	gint32 Bias;						// 4
	guint16 StandardName[32];			// 64
	EasSystemTime StandardDate;			// 16
	gint32 StandardBias;				// 4
	guint16 DaylightName[32];			// 64
	EasSystemTime DaylightDate;			// 16
	gint32 DaylightBias;				// 4
} __attribute__((packed)) EasTimeZone;	// 172

compile_time_assert((sizeof(EasTimeZone) == 172), EasTimeZone_not_expected_size);

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
				else if(g_strcmp0(name, "Attendees") == 0)
				{
					g_debug("Found attendees sequence");					
					xmlNode* attendeeNode = NULL;
					for (attendeeNode = n->children; attendeeNode; attendeeNode = attendeeNode->next)
					{
						g_debug("Found attendee");				
						// TODO make all this string handling more efficient if poss
						GString* attparams = g_string_new("");
						GString* cal_address = g_string_new("mailto:");						
							
						xmlNode *subNode = NULL;
						for (subNode = attendeeNode->children; subNode; subNode = subNode->next)
						{
							if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0(subNode->name, "Attendee_Email"))
							{
								xmlChar* email = xmlNodeGetContent(subNode);
								g_debug("found attendee email");
								//mailto
								cal_address = g_string_append(cal_address, email);
								g_free(email);
								g_debug("cal_address = %s", cal_address->str);
							}								
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0(subNode->name, "Attendee_Name"))
							{
								xmlChar* name = xmlNodeGetContent(subNode);								
								g_debug("found name");
								//cnparam
								attparams = g_string_append(attparams, ";");
								attparams = g_string_append(attparams, "CN=");
								attparams = g_string_append(attparams, name);	
								g_free(name);
								g_debug("attparams = %s", attparams->str);
							}
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0(subNode->name, "Attendee_Status"))
							{
								g_debug("found status");																 
								xmlChar* status_as_string = xmlNodeGetContent(subNode);
								gchar *status_ical;
								guint status = atoi(status_as_string);
								switch(status)
								{
									// TODO create an enum for these values
									case 0: // Response unknown
									case 5: // Not responded
									{
										status_ical = strdup("NEEDS-ACTION");
									}
									break;
									case 2: // Tentative
									{
										status_ical = strdup("TENTATIVE");
									}
									break;									
									case 3: // Accept
									{
										status_ical = strdup("ACCEPTED");
									}
									break;	
									case 4: // Decline
									{
										status_ical = strdup("DECLINED");
									}
									break;									
									default:
									{
										g_warning("unrecognised attendee status received");
									}
								}// end switch status
								//partstatparam
								attparams = g_string_append(attparams, ";");
								attparams = g_string_append(attparams, "PARTSTAT=");
								attparams = g_string_append(attparams, status_ical);
								g_free(status_as_string);
								g_free(status_ical);
								g_debug("attparams = %s", attparams->str);								
							}
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0(subNode->name, "Attendee_Type"))
							{
								g_debug("found type");								
								xmlChar* type_as_string = xmlNodeGetContent(subNode);
								guint type = atoi(type_as_string);
								gchar *type_ical;
								switch(type)
								{
									// TODO create an enum for these values
									case 1: //Required
									{
										type_ical = g_strdup("REQ-PARTICIPANT");
									}
									break;
									case 2: //Optional
									{
										type_ical = g_strdup("OPT-PARTICIPANT");
									}
									break;
									case 3: //Resource
									{
										type_ical = g_strdup("NON-PARTICIPANT");
									}
									break;
									default:
									{
										g_warning("unrecognised attendee type received");
									}
								}// end switch type
								
								//roleparam
								attparams = g_string_append(attparams, ";");
								attparams = g_string_append(attparams, "ROLE=");
								attparams = g_string_append(attparams, type_ical);
								g_free(type_as_string);
								g_free(type_ical);
								g_debug("attparams = %s", attparams->str);
							}		
							
						}// end for subNodes	

						gchar *attendee = g_strconcat("ATTENDEE", attparams->str, NULL);

						// Adding to VEVENT. EAS doesn't appear to support EMAIL ALARMS, so not adding to VALARM
						_util_append_prop_string_to_list(&vevent, attendee, cal_address->str);	

						// Free the strings, including the character buffer
						g_string_free(attparams, TRUE);
						g_string_free(cal_address, TRUE);
					}//end for (attendee)
					
				}// end else if (attendees)
				else if (g_strcmp0(name, "TimeZone") == 0)
				{
					xmlChar *timeZoneB64 = xmlNodeGetContent(n);
					EasTimeZone tz;
					WB_UTINY *timeZone = NULL;
					WB_LONG tz_length = 0;

					// Expect timeZoneB64 to be NULL terminated
					tz_length = wbxml_base64_decode(timeZoneB64, -1, &timeZone);
					xmlFree(timeZoneB64);

					// TODO Check decode of timezone for endianess problems

					if (tz_length == sizeof(tz))
					{
						gchar *result = NULL;
						memcpy(&tz, timeZone, sizeof(tz));

						// @@WARNING: TZID Mandatory, but no equivalent supplied from AS
						result = g_utf16_to_utf8((const gunichar2*)tz.StandardName, (sizeof(tz.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
						g_warning("Using TimeZone.StandardName as the mandatory iCal TZID as ActiveSync has no equivalent data field");
						_util_append_prop_string_to_list(&vtimezone, "TZID", result);
						g_free(result); result = NULL;

						_util_append_prop_string_to_list(&vtimezone, "BEGIN", "STANDARD");

						// YEAR MONTH DAY 'T' HOUR MINUTE SECONDS
						result = g_strdup_printf("%04u%02u%02uT%02u%02u%02u",
												tz.StandardDate.Year, 
												tz.StandardDate.Month,
												tz.StandardDate.Day,
												tz.StandardDate.Hour,
												tz.StandardDate.Minute,
												tz.StandardDate.Second);

						_util_append_prop_string_to_list(&vtimezone, "DTSTART", result); // StandardDate
						g_free(result); result = NULL;

						/* The property value is a signed numeric indicating the 
						 * number of hours and possibly minutes from UTC. Positive 
						 * numbers represent time zones east of the prime meridian, 
						 * or ahead of UTC. Negative numbers represent time zones 
						 * west of the prime meridian, or behind UTC.
						 * E.g. TZOFFSETFROM:-0500 or TZOFFSETFROM:+1345
						 */
						result = g_strdup_printf("%+03d%02d", tz.Bias/60, tz.Bias%60);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETFROM", result); // Bias
						g_free(result); result = NULL;

						result = g_strdup_printf("%+03d%02d", tz.StandardBias/60, tz.StandardBias%60);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETTO", result);   // StandardBias
						g_free(result); result = NULL;

						result = g_utf16_to_utf8((const gunichar2*)tz.StandardName, (sizeof(tz.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
						_util_append_prop_string_to_list(&vtimezone, "TNAME", result);        // StandardName
						g_free(result); result = NULL;
						_util_append_prop_string_to_list(&vtimezone, "END", "STANDARD");

						_util_append_prop_string_to_list(&vtimezone, "BEGIN", "DAYLIGHT");
						result = g_strdup_printf("%04u%02u%02uT%02u%02u%02u", 
							                     tz.DaylightDate.Year, 
							                     tz.DaylightDate.Month,
					                             tz.DaylightDate.Day,
					                             tz.DaylightDate.Hour,
					                             tz.DaylightDate.Minute,
					                             tz.DaylightDate.Second);
						_util_append_prop_string_to_list(&vtimezone, "DTSTART", result); // DaylightDate
						g_free(result); result = NULL;

						result = g_strdup_printf("%+03d%02d", tz.Bias/60, tz.Bias%60);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETFROM", result);  // Bias
						g_free(result); result = NULL;

						result = g_strdup_printf("%+03d%02d", tz.DaylightBias/60, tz.DaylightBias%60);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETTO", result);    // DaylightBias
						g_free(result); result = NULL;

						result = g_utf16_to_utf8((const gunichar2 *)tz.DaylightName, (sizeof(tz.DaylightName)/sizeof(guint16)), NULL, NULL, NULL);
						_util_append_prop_string_to_list(&vtimezone, "TNAME", result);         // DaylightName
						g_free(result); result = NULL;
						_util_append_prop_string_to_list(&vtimezone, "END", "DAYLIGHT");
					} // tz_length == sizeof(tz)
					else
					{
						g_critical("TimeZone BLOB did not match sizeof(EasTimeZone)");
					}
					g_free(timeZone);
				}

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


/**
 * Parse the STANDARD and DAYLIGHT subcomponents of VTIMEZONE.
 * Using one function for both as their formats are identical.
 */
static void _util_process_vtimezone_subcomponent(icalcomponent* subcomponent, EasTimeZone* timezone, icalcomponent_kind type)
{
	if (subcomponent)
	{
		// Determine whether we've been passed a STANDARD or DAYLIGHT component
		// and get a pointer to the appropriate time struct
		const gboolean isStandardTime = (type == ICAL_XSTANDARD_COMPONENT);
		EasSystemTime* easTimeStruct = (isStandardTime ? (&(timezone->StandardDate)) : (&(timezone->DaylightDate)));

		// Get the properties we're interested in. Note RRULE is optional but the rest are mandatory
		icalproperty* dtStart = icalcomponent_get_first_property(subcomponent, ICAL_DTSTART_PROPERTY);
		icalproperty* rrule = icalcomponent_get_first_property(subcomponent, ICAL_RRULE_PROPERTY);
		icalproperty* tzOffsetTo = icalcomponent_get_first_property(subcomponent, ICAL_TZOFFSETTO_PROPERTY);
		icalproperty* tzOffsetFrom = icalcomponent_get_first_property(subcomponent, ICAL_TZOFFSETFROM_PROPERTY);

		// Get the values of the properties
		// Note: icalproperty_get_tzoffsetto() and icalproperty_get_tzofsetfrom() return offsets as seconds.
		const icaltimetype dtStartValue = icalproperty_get_dtstart(dtStart);
		const int tzOffsetToValueMins = icalproperty_get_tzoffsetto(tzOffsetTo) / SECONDS_PER_MINUTE;
		const int tzOffsetFromValueMins = icalproperty_get_tzoffsetfrom(tzOffsetFrom) / SECONDS_PER_MINUTE;

		struct icalrecurrencetype rruleValue;
		if (rrule)
		{
			rruleValue = icalproperty_get_rrule(rrule);
		}
		else
		{
			icalrecurrencetype_clear(&rruleValue);
		}
		
		if (isStandardTime)
		{
			// Calculate the EAS bias value. Bias represents a UTC offset, but expressed the opposite way
			// round to the usual notation. In EAS, Bias is the offset added to the local timezone to get to
			// UTC, whereas the conventional notation is the offset added to UTC to get to the local time.
			// For example, Pacific Standard Time is at UTC-8 (8 hours behind UTC - deduct 8 from a UTC time to
			// get the corresponding PST time), but in EAS that's expressed as a Bias of +480 minutes (i.e. add
			// 8 hours to a PST time to get back to UTC). Likewise, Central European Time (UTC+1) has a Bias
			// value of -60. See http://msdn.microsoft.com/en-us/library/ms725481(v=vs.85).aspx for more details.
			timezone->Bias = -1 * tzOffsetToValueMins;

			// Standard bias is always 0 (it's the Standard time's offset from the Bias)
			timezone->StandardBias = 0; // Always zero
		}
		else // It's daylight time
		{
			// As with the bias above, this value is inverted from our usual understanding of it. e.g. If a
			// daylight saving phase adds 1 hour to the standard phase, the DaylightBias value is -60. Note
			// that DaylightBias and StandardBias are the additional offsets *relative to the base Bias value*
			// (rather than absolute offsets from UTC). We can calculate the daylight bias easily as follows:
			timezone->DaylightBias = tzOffsetFromValueMins - tzOffsetToValueMins;
		}

		// Handle recurrence information if present
		if (rrule)
		{
			// Assuming FREQ=YEARLY - TODO: check this is safe...
			short byMonth = rruleValue.by_month[0];

			short byDayRaw = rruleValue.by_day[0];
			icalrecurrencetype_weekday byDayWeekday = icalrecurrencetype_day_day_of_week(byDayRaw);
			/** 0 == any of day of week. 1 == first, 2 = second, -2 == second to last, etc */
			int byDayPosition = icalrecurrencetype_day_position(byDayRaw);

			easTimeStruct->Year = 0; // Always 0 if we have recurrence
			easTimeStruct->Month = byMonth;

			// The day is the tricky bit...
			// Both formats use this to represent nth occurrence of a day in the month
			// (ie. it's NOT an absolute date). However, iCal supports this notation:
			//    +1 = First of the month
			//    +2 = 2nd in the month
			//    -1 = Last in the month
			//    -2 = 2nd-to-last in the month
			//    etc.
			//
			// Whilst EAS uses:
			//     1 = First in month
			//     2 = 2nd in month
			//     ...
			//     5 = Last in month, even if there are only 4 occurrences
			//
			// In other words, EAS cannot encode "2nd-to-last in month" etc.
			// The best we can do is to add negative iCal values to 6, so
			//    iCal -1 => EAS 5
			//    iCal -2 => EAS 4
			// etc. even though 2nd-to-last isn't always 4th.

			// Every day occurs in a month at least 4 times (a non-leap February has 4 full weeks)
			// and no more than 5 times (5 full weeks = 35 days, longer than any month). So the day
			// position must be in the range +/-(1..5)
			g_assert((-5 <= byDayPosition && byDayPosition <= -1) || (1 <= byDayPosition && byDayPosition <= 5));
			if (byDayPosition > 0)
			{
				easTimeStruct->Day = byDayPosition;
			}
			else // byDayPosition is negative
			{
				// Convert -1 to 5, -2 to 4, etc. (see above for reason why)
				easTimeStruct->Day = 6 + byDayPosition;
			}

			// Don't want to rely on enum values in icalrecurrencetype_weekday so use a switch statement to set the DayOfWeek
			switch (byDayWeekday)
			{
				case ICAL_SUNDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 0;
					break;
				case ICAL_MONDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 1;
					break;
				case ICAL_TUESDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 2;
					break;
				case ICAL_WEDNESDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 3;
					break;
				case ICAL_THURSDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 4;
					break;
				case ICAL_FRIDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 5;
					break;
				case ICAL_SATURDAY_WEEKDAY:
					easTimeStruct->DayOfWeek = 6;
					break;
			}

			// Date set. Time is set below...
		}
		else // No recurrence information: just a one-off time change, so we set an absolute date value for EAS
		{
			easTimeStruct->Year = dtStartValue.year;
			easTimeStruct->Month = dtStartValue.month; // Both use 1 (Jan) ... 12 (Dec)
			easTimeStruct->Day = dtStartValue.day;
			// Date set. Time is set below...
		}

		// Set the time fields
		easTimeStruct->Hour = dtStartValue.hour;
		easTimeStruct->Minute = dtStartValue.minute;
		easTimeStruct->Second = dtStartValue.second;
		easTimeStruct->Millisecond = 0;

		// Destroy the property objects
		icalproperty_free(dtStart);
		icalproperty_free(tzOffsetFrom);
		icalproperty_free(tzOffsetTo);
		if (rrule)
		{
			icalproperty_free(rrule);
		}
	}
}


/**
 * Parse a VTIMEZONE component_data
 */
static void _util_process_vtimezone_component(icalcomponent* vtimezone, xmlNodePtr app_data)
{
	if (vtimezone)
	{
		EasTimeZone timezoneStruct;

		// Only one property in a VTIMEZONE: the TZID
		icalproperty* tzid = icalcomponent_get_first_property(vtimezone, ICAL_TZID_PROPERTY);
		if (tzid)
		{
			// Get the ASCII value from the iCal
			const gchar* tzidValue8 = (const gchar*)icalproperty_get_value_as_string(tzid);

			// Convert to Unicode, max. 32 chars (including the trailing 0)
			gunichar2* tzidValue16 = g_utf8_to_utf16(tzidValue8, 31, NULL, NULL, NULL);

			// Copy this into the EasTimeZone struct as both StandardName and DaylightName
			memcpy(&(timezoneStruct.StandardName), tzidValue16, 64); // 32 Unicode chars = 64 bytes
			memcpy(&(timezoneStruct.DaylightName), tzidValue16, 64);

			g_free(tzidValue8);
			g_free(tzidValue16);
		}

		// Now process the STANDARD and DAYLIGHT subcomponents
		icalcomponent* subcomponent;
		for (subcomponent = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT);
		     subcomponent;
		     subcomponent = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT))
		{
			_util_process_vtimezone_subcomponent(subcomponent, &timezoneStruct, icalcomponent_isa(subcomponent));
		}

		// Write the timezone into the XML, base64-encoded
		const int rawStructSize = sizeof(EasTimeZone);
		WB_UTINY* timezoneBase64 = wbxml_base64_encode((const WB_UTINY*)(&timezoneStruct), rawStructSize);
		xmlNewTextChild(app_data, NULL, "calendar:Timezone", timezoneBase64);
		g_free(timezoneBase64);
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
		_util_process_vtimezone_component(icalcomponent_get_first_component(ical, ICAL_VTIMEZONE_COMPONENT), app_data);
		icalcomponent* vevent = icalcomponent_get_first_component(ical, ICAL_VEVENT_COMPONENT);
		_util_process_vevent_component(vevent, app_data);
		_util_process_valarm_component(icalcomponent_get_first_component(vevent, ICAL_VALARM_COMPONENT), app_data);

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

