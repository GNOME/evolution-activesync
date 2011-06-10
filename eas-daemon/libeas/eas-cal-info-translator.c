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
const gint SECONDS_PER_MINUTE               = 60;
const gint MINUTES_PER_HOUR                 = 60;
const gint MINUTES_PER_DAY                  = 60 * 24;
const gint MINUTES_PER_WEEK                 = 60 * 24 * 7;
const gint EPOCH_START_YEAR					= 1970;

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
	GString* line_for_folding = NULL;
	guint next_line_start = 0;

	// TODO: there's a lot of string copying going on in this function.
	// Need to think if there's a more efficient way of doing this.


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
	
	line_for_folding = g_string_new(line_with_newlines_handled);
	// Use an index to mark the position the next line should be started at
	next_line_start = ICAL_MAX_LINE_LENGTH;
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
 * \brief Private function to decode an EasSystemTime struct containing details of a recurring
 *        timezone change pattern, modify it to a format iCalendar expects and create the
 *        required RRULE property value.
 *
 * \param date        Pointer to the EasSystemTime struct. This will be modified in this
 *                    function.
 * \param rruleValue  Pointer to a NULL buffer. This will be instantiated in this function.
 *                    The calling code is responsible for freeing the buffer after use
 *                    (using g_free())
 */
static void _util_convert_relative_timezone_date(EasSystemTime* date, gchar** rruleValue)
{
	GDate* recurrenceStartDate = NULL;
	GDateWeekday weekday;
	gint occurrence;
	gint byDayNth;
	gchar* byDayName = NULL;
	
	// DTSTART needs to identify the first date of the recurring pattern in some
	// year previous to any items in the iCalendar. We'll just use 1970 as the
	// start of the UNIX epoch.
	date->Year = EPOCH_START_YEAR;

	// date->DayOfWeek will give us the day to repeat on
	// date->Day will give us the nth occurrence in the month of this day
	// (where 5 == last, even if there are only 4)

	// Start at the beginning of the month
	recurrenceStartDate = g_date_new_dmy(1, date->Month, date->Year);

	// Seek for the first occurrence of the day.
	// SYSTEMTIME's wDayOfWeek has 0 = Sunday, 1 = Monday,...     http://msdn.microsoft.com/en-us/library/ms724950(v=vs.85).aspx
	// GDateWeekDay has 0 = G_DATE_BAD_WEEKDAY, 1 = Monday,...    http://developer.gnome.org/glib/stable/glib-Date-and-Time-Functions.html#GDateWeekday
	if (date->DayOfWeek == 0)
	{
		date->DayOfWeek = 7;
		// Now our DayOfWeek matches GDateWeekDay
	}
	weekday = (GDateWeekday)date->DayOfWeek;
	while (g_date_get_weekday(recurrenceStartDate) != weekday)
	{
		g_date_add_days (recurrenceStartDate, 1);
	}

	// Now we've got the first occurence of weekday in the right month in 1970.
	// Now seek for the nth occurrence (where 5th = last, even if there are only 4)
	occurrence = 1;
	while (occurrence++ < date->Day)
	{
		g_date_add_days(recurrenceStartDate, 7);

		// Check we havn't overrun the end of the month
		// (If we have, just roll back and we're done: we're on the last occurrence)
		if (g_date_get_month(recurrenceStartDate) != date->Month)
		{
			g_date_subtract_days(recurrenceStartDate, 7);
			break;
		}
	}

	// Now build the RRULE value before modifying date->Day
	byDayNth = date->Day;
	if (byDayNth == 5)
	{
		byDayNth = -1;
	}
	
	switch (weekday)
	{
		case G_DATE_MONDAY:
			byDayName = g_strdup("MO");
			break;
		case G_DATE_TUESDAY:
			byDayName = g_strdup("TU");
			break;
		case G_DATE_WEDNESDAY:
			byDayName = g_strdup("WE");
			break;
		case G_DATE_THURSDAY:
			byDayName = g_strdup("TH");
			break;
		case G_DATE_FRIDAY:
			byDayName = g_strdup("FR");
			break;
		case G_DATE_SATURDAY:
			byDayName = g_strdup("SA");
			break;
		case G_DATE_SUNDAY:
			byDayName = g_strdup("SU");
			break;
		case G_DATE_BAD_WEEKDAY:
		default:
			g_warning("Unknown weekday");
			break;
	}
	
	*rruleValue = g_strdup_printf("FREQ=YEARLY;BYDAY=%d%s;BYMONTH=%d", byDayNth, byDayName, date->Month);
	g_free(byDayName);
	byDayName = NULL;

	// Finally, we can set the day in the SYSTEMTIME struct to be the start day of the
	// recurrence sequence.
	date->Day = g_date_get_day(recurrenceStartDate);
}


/**
 * \brief Modify a date/time property name (DTSTART, DTEND or DTSTAMP) to add a TZID (timezone ID) attribute if required
 *
 * \param propertyName      Pointer to the property name string (e.g. "DTSTART"). This may be modified by
 *                          this function. The calling code keeps the responsibility for freeing the
 *                          memory after use.
 * \param propertyValue     The value for this property (i.e. a date/time in ISO format)
 * \param tzid              The TZID field from the VTIMEZONE component (if present). If none was present,
 *                          pass either NULL or an empty string.
 */
static void _util_add_timezone_to_property_name(gchar** propertyName, const gchar* propertyValue, const gchar* tzid)
{
	// We add the TZID attribute if:
	//  - we have an attribute value to add!
	//  - the time value doesn't end in Z (as this indicates a UTC time, so no timezone needs to be specified)
	if (tzid && strlen(tzid) && !g_str_has_suffix(propertyValue, "Z"))
	{
		gchar* temp = g_strdup_printf("%s;TZID=\"%s\"", *propertyName, tzid);
		g_free(*propertyName);
		*propertyName = temp;
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
	gchar* result = NULL;

	// TODO: Oops! I only found libical after I'd implemented this.
	// We should switch to libical - it will make further development a lot easier and more robust.
	
	if (node && (node->type == XML_ELEMENT_NODE) && (!g_strcmp0((char*)(node->name), "ApplicationData")))
	{
		xmlNodePtr n = node;
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

		// Variable to store the TZID value when decoding a <calendar:Timezone> element
		// so we can use it in the rest of the iCal's date/time fields.
		// TODO: check <calendar:Timezone> always occurs at the top of the calendar item.
		gchar *tzid = (gchar*)"";
		GString* ical_buf = NULL;
		GSList* item = NULL;
		EasCalInfo* cal_info = NULL;

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
					gchar* propertyName = g_strdup("DTSTART");
					gchar* propertyValue = (gchar*)xmlNodeGetContent(n);
					_util_add_timezone_to_property_name(&propertyName, propertyValue, tzid);
					_util_append_prop_string_to_list(&vevent, propertyName, propertyValue);
					g_free(propertyName);
				    g_free(propertyValue);
				}
				else if (g_strcmp0(name, "EndTime") == 0)
				{
					gchar* propertyName = g_strdup("DTEND");
					gchar* propertyValue = (gchar*)xmlNodeGetContent(n);
					_util_add_timezone_to_property_name(&propertyName, propertyValue, tzid);
					_util_append_prop_string_to_list(&vevent, propertyName, propertyValue);
					g_free(propertyName);
				    g_free(propertyValue);
				}
				else if (g_strcmp0(name, "DtStamp") == 0)
				{
					gchar* propertyName = g_strdup("DTSTAMP");
					gchar* propertyValue = (gchar*)xmlNodeGetContent(n);
					_util_add_timezone_to_property_name(&propertyName, propertyValue, tzid);
					_util_append_prop_string_to_list(&vevent, propertyName, propertyValue);
					g_free(propertyName);
				    g_free(propertyValue);
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
						if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar *)subNode->name, "Data"))
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
						xmlChar* category = xmlNodeGetContent(catNode);
						if (categories->len > 0)
						{
							categories = g_string_append(categories, ",");
						}

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
					xmlNode* attendeeNode = NULL;
					g_debug("Found attendees sequence");
					for (attendeeNode = n->children; attendeeNode; attendeeNode = attendeeNode->next)
					{
						// TODO make all this string handling more efficient if poss
						GString* attparams = g_string_new("");
						GString* cal_address = g_string_new("mailto:");
						xmlNode *subNode = NULL;
						gchar *attendee = NULL;
						
						g_debug("Found attendee");

						for (subNode = attendeeNode->children; subNode; subNode = subNode->next)
						{
							if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar *)subNode->name, "Attendee_Email"))
							{
								xmlChar* email = xmlNodeGetContent(subNode);
								g_debug("found attendee email");
								//mailto
								cal_address = g_string_append(cal_address, (gchar *)email);
								g_free(email);
								g_debug("cal_address = %s", cal_address->str);
							}								
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar *)subNode->name, "Attendee_Name"))
							{
								xmlChar* name = xmlNodeGetContent(subNode);
								g_debug("found name");
								//cnparam
								attparams = g_string_append(attparams, ";");
								attparams = g_string_append(attparams, "CN=");
								attparams = g_string_append(attparams, (gchar *)name);	
								g_free(name);
								g_debug("attparams = %s", attparams->str);
							}
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar *)subNode->name, "Attendee_Status"))
							{
								xmlChar* status_as_string = xmlNodeGetContent(subNode);
								gchar *status_ical = NULL;
								guint status = atoi((gchar *)status_as_string);
								
								g_debug("found status");
								
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
							else if(subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar *)subNode->name, "Attendee_Type"))
							{
								xmlChar* type_as_string = xmlNodeGetContent(subNode);
								guint type = atoi((gchar *)type_as_string);
								gchar *type_ical = NULL;
								
								g_debug("found type");
								
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

						attendee = g_strconcat("ATTENDEE", attparams->str, NULL);

						// Adding to VEVENT. EAS doesn't appear to support EMAIL ALARMS, so not adding to VALARM
						_util_append_prop_string_to_list(&vevent, attendee, cal_address->str);	

						// Free the strings, including the character buffer
						g_string_free(attparams, TRUE);
						g_string_free(cal_address, TRUE);
					}//end for (attendee)
				}// end else if (attendees)
				else if (g_strcmp0(name, "TimeZone") == 0)
				{
					xmlChar* timeZoneBase64Buffer = xmlNodeGetContent(n);
					gsize timeZoneRawBytesSize = 0;
					guchar* timeZoneRawBytes = g_base64_decode((const gchar*)timeZoneBase64Buffer, &timeZoneRawBytesSize);
					EasTimeZone timeZoneStruct;
					xmlFree(timeZoneBase64Buffer);

					// TODO Check decode of timezone for endianess problems

					if (timeZoneRawBytesSize == sizeof(EasTimeZone))
					{
						gchar* value = NULL;
						memcpy(&timeZoneStruct, timeZoneRawBytes, timeZoneRawBytesSize);
						g_free(timeZoneRawBytes);
						timeZoneRawBytes = NULL;

						// Calculate the timezone offsets. See _util_process_vtimezone_subcomponent()
						// comments for a full explanation of how EAS Bias relates to iCal UTC offsets
						{ // @@WARNING - Start of scope
						const gint32 standardUtcOffsetMins = -1 * timeZoneStruct.Bias;
						const gint32 daylightUtcOffsetMins = -1 * (timeZoneStruct.Bias + timeZoneStruct.DaylightBias);
						gchar* standardUtcOffsetStr = g_strdup_printf("%+03d%02d", standardUtcOffsetMins / MINUTES_PER_HOUR, standardUtcOffsetMins % MINUTES_PER_HOUR);
						gchar* daylightUtcOffsetStr = g_strdup_printf("%+03d%02d", daylightUtcOffsetMins / MINUTES_PER_HOUR, daylightUtcOffsetMins % MINUTES_PER_HOUR);
						gchar* rruleValue = NULL;
						
						// Using StandardName as the TZID
						// (Doesn't matter if it's not an exact description: this field is only used internally
						// during iCalendar encoding/decoding)
						// Note: using tzid here rather than value, as we need it elsewhere in this function
						tzid = g_utf16_to_utf8((const gunichar2*)timeZoneStruct.StandardName,
						                       (sizeof(timeZoneStruct.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
						// If no StandardName was supplied, we can just use a temporary name instead.
						// No need to support more than one: the EAS calendar will only have one Timezone
						// element. And no need to localise, as it's only used internally.
						if (tzid == NULL || strlen(tzid) == 0)
						{
							tzid = g_strdup("Standard Timezone");
						}
						_util_append_prop_string_to_list(&vtimezone, "TZID", tzid);

						
						// STANDARD component
						
						_util_append_prop_string_to_list(&vtimezone, "BEGIN", "STANDARD");

						// If timeZoneStruct.StandardDate.Year == 0 we need to convert it into
						// the start date of a recurring sequence, and add an RRULE.
						if (timeZoneStruct.StandardDate.Year == 0)
						{
							_util_convert_relative_timezone_date(&timeZoneStruct.StandardDate, &rruleValue);
						}

						// YEAR MONTH DAY 'T' HOUR MINUTE SECONDS
						value = g_strdup_printf("%04u%02u%02uT%02u%02u%02u",
												timeZoneStruct.StandardDate.Year,
												timeZoneStruct.StandardDate.Month,
												timeZoneStruct.StandardDate.Day,
												timeZoneStruct.StandardDate.Hour,
												timeZoneStruct.StandardDate.Minute,
												timeZoneStruct.StandardDate.Second);
						_util_append_prop_string_to_list(&vtimezone, "DTSTART", value); // StandardDate
						g_free(value); value = NULL;

						// Add the RRULE (if required)
						if (rruleValue != NULL)
						{
							_util_append_prop_string_to_list(&vtimezone, "RRULE", rruleValue);
							g_free(rruleValue);
							rruleValue = NULL;
						}
						
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETFROM", daylightUtcOffsetStr);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETTO", standardUtcOffsetStr);

						value = g_utf16_to_utf8((const gunichar2*)timeZoneStruct.StandardName, (sizeof(timeZoneStruct.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
						if (value)
						{
							if (strlen(value))
							{
								_util_append_prop_string_to_list(&vtimezone, "TNAME", value);        // StandardName
							}
							g_free(value); value = NULL;
						}
						_util_append_prop_string_to_list(&vtimezone, "END", "STANDARD");


						// DAYLIGHT component

						_util_append_prop_string_to_list(&vtimezone, "BEGIN", "DAYLIGHT");

						// If timeZoneStruct.StandardDate.Year == 0 we need to convert it into
						// the start date of a recurring sequence, and add an RRULE.
						if (timeZoneStruct.DaylightDate.Year == 0)
						{
							_util_convert_relative_timezone_date(&timeZoneStruct.DaylightDate, &rruleValue);
						}

						
						value = g_strdup_printf("%04u%02u%02uT%02u%02u%02u", 
							                    timeZoneStruct.DaylightDate.Year,
							                    timeZoneStruct.DaylightDate.Month,
					                            timeZoneStruct.DaylightDate.Day,
					                            timeZoneStruct.DaylightDate.Hour,
					                            timeZoneStruct.DaylightDate.Minute,
					                            timeZoneStruct.DaylightDate.Second);
						_util_append_prop_string_to_list(&vtimezone, "DTSTART", value); // DaylightDate
						g_free(value); value = NULL;

						// Add the RRULE (if required)
						if (rruleValue != NULL)
						{
							_util_append_prop_string_to_list(&vtimezone, "RRULE", rruleValue);
							g_free(rruleValue);
							rruleValue = NULL;
						}
						
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETFROM", standardUtcOffsetStr);
						_util_append_prop_string_to_list(&vtimezone, "TZOFFSETTO", daylightUtcOffsetStr);

						value = g_utf16_to_utf8((const gunichar2 *)timeZoneStruct.DaylightName, (sizeof(timeZoneStruct.DaylightName)/sizeof(guint16)), NULL, NULL, NULL);
						if (value)
						{
							if (strlen(value))
							{
								_util_append_prop_string_to_list(&vtimezone, "TNAME", value);   // DaylightName
							}
							g_free(value); value = NULL;
						}
						_util_append_prop_string_to_list(&vtimezone, "END", "DAYLIGHT");

						g_free(standardUtcOffsetStr);
						g_free(daylightUtcOffsetStr);
						} // @@WARNING - End of Scope
					} // timeZoneRawBytesSize == sizeof(timeZoneStruct)
					else
					{
						g_critical("TimeZone BLOB did not match sizeof(EasTimeZone)");
					}
				}

				// TODO: handle Recurrence element
				// TODO: handle Exceptions element
			}
		}

		// TODO: Think of a way to pre-allocate a sensible size for the buffer
		ical_buf = g_string_new("");
		_util_append_line_to_ical_buffer(&ical_buf, "BEGIN:VCALENDAR", FALSE);

		// Add the VCALENDAR properties first
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
		cal_info = eas_cal_info_new();
		cal_info->icalendar = g_string_free(ical_buf, FALSE); // Destroy the GString object and pass its buffer (with ownership) to cal_info
		cal_info->server_id = (gchar*)server_id;
		if (!eas_cal_info_serialise(cal_info, &result))
		{
			// TODO: log error
			result = NULL;
		}

		g_object_unref(cal_info);

		if (tzid)
		{
			g_free(tzid);
		}
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
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Subject", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTSTAMP_PROPERTY:
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:DtStamp", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTSTART_PROPERTY:
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:StartTime", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_DTEND_PROPERTY:
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:EndTime", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_LOCATION_PROPERTY:
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Location", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_UID_PROPERTY:
					xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:UID", (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
				case ICAL_CLASS_PROPERTY:
					{
						const gchar* value = (const gchar*)icalproperty_get_value_as_string(prop);
						if (g_strcmp0(value, "CONFIDENTIAL") == 0)
						{
							xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Sensitivity", (const xmlChar*)"3"); // Confidential
						}
						else if (g_strcmp0(value, "PRIVATE") == 0)
						{
							xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Sensitivity", (const xmlChar*)"2"); // Private
						}
						else // PUBLIC
						{
							// iCalendar doesn't distinguish between 0 (Normal) and 1 (Personal)
							xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Sensitivity", (const xmlChar*)"0");
						}
					}
					break;
				case ICAL_TRANSP_PROPERTY:
					{
						const gchar* value = (const gchar*)icalproperty_get_value_as_string(prop);
						if (g_strcmp0(value, "TRANSPARENT") == 0)
						{
							xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:BusyStatus", (const xmlChar*)"0"); // Free
						}
						else // OPAQUE
						{
							// iCalendar doesn't distinguish between 1 (Tentative), 2 (Busy), 3 (Out of Office)
							xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:BusyStatus", (const xmlChar*)"2"); // Busy
						}
					}
					break;
				case ICAL_CATEGORIES_PROPERTY:
					{
						if (categories == NULL)
						{
							categories = xmlNewChild(app_data, NULL, (const xmlChar*)"calendar:Categories", NULL);
						}

						xmlNewTextChild(categories, NULL, (const xmlChar*)"calendar:Category", (const xmlChar*)icalproperty_get_value_as_string(prop));
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
		// Just need to get the TRIGGER property
		icalproperty* prop = icalcomponent_get_first_property(valarm, ICAL_TRIGGER_PROPERTY);
		g_debug("Processing VALARM...");
		if (prop)
		{
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
			g_debug("Processing TRIGGER...");

			g_snprintf(minutes_buf, 6, "%d", minutes);

			xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Reminder", (const xmlChar*)minutes_buf);
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
				case ICAL_NO_WEEKDAY:
				default:
					g_warning("Unknown byDayWeekday");
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
		icalcomponent* subcomponent;
		gchar* timezoneBase64 = NULL;
			
		// Only one property in a VTIMEZONE: the TZID
		icalproperty* tzid = icalcomponent_get_first_property(vtimezone, ICAL_TZID_PROPERTY);
		if (tzid)
		{
			// Get the ASCII value from the iCal
			gchar* tzidValue8 = (gchar*)icalproperty_get_value_as_string(tzid);

			// Convert to Unicode, max. 32 chars (including the trailing 0)
			gunichar2* tzidValue16 = g_utf8_to_utf16(tzidValue8, 31, NULL, NULL, NULL);

			// Copy this into the EasTimeZone struct as both StandardName and DaylightName
			memcpy(&(timezoneStruct.StandardName), tzidValue16, 64); // 32 Unicode chars = 64 bytes
			memcpy(&(timezoneStruct.DaylightName), tzidValue16, 64);

			g_free(tzidValue8);
			g_free(tzidValue16);
		}

		// Now process the STANDARD and DAYLIGHT subcomponents
		for (subcomponent = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT);
		     subcomponent;
		     subcomponent = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT))
		{
			_util_process_vtimezone_subcomponent(subcomponent, &timezoneStruct, icalcomponent_isa(subcomponent));
		}

		// Write the timezone into the XML, base64-encoded
		timezoneBase64 = g_base64_encode((const guchar *)(&timezoneStruct), sizeof(EasTimeZone));
		xmlNewTextChild(app_data, NULL, (const xmlChar*)"calendar:Timezone", (const xmlChar*)timezoneBase64);
		g_free(timezoneBase64);
	}
}


/**
 *
 */
gboolean eas_cal_info_translator_parse_request(xmlDocPtr doc, xmlNodePtr app_data, EasCalInfo* cal_info)
{
	gboolean success = FALSE;

	icalcomponent* ical = NULL;
	if (doc &&
	    app_data &&
	    cal_info &&
	    (app_data->type == XML_ELEMENT_NODE) &&
	    (g_strcmp0((char*)(app_data->name), "ApplicationData") == 0) &&
	    (ical = icalparser_parse_string(cal_info->icalendar)) &&
	    (icalcomponent_isa(ical) == ICAL_VCALENDAR_COMPONENT))
	{
		// Process the components of the VCALENDAR
		icalcomponent* vevent = NULL;
		_util_process_vtimezone_component(icalcomponent_get_first_component(ical, ICAL_VTIMEZONE_COMPONENT), app_data);
		vevent = icalcomponent_get_first_component(ical, ICAL_VEVENT_COMPONENT);
		_util_process_vevent_component(vevent, app_data);
		_util_process_valarm_component(icalcomponent_get_first_component(vevent, ICAL_VALARM_COMPONENT), app_data);

		// DEBUG output
		{ // @@WARNING - Start of Scope
		xmlChar* dump_buffer;
		int dump_buffer_size;
		xmlIndentTreeOutput = 1;
		xmlDocDumpFormatMemory(doc, &dump_buffer, &dump_buffer_size, 1);
		g_debug("XML DOCUMENT DUMPED:\n%s", dump_buffer);
		xmlFree(dump_buffer);
		} // @@WARNING - End of Scope

		success = TRUE;
	}

	if (ical)
	{
		icalcomponent_free(ical);
	}

	return success;
}

