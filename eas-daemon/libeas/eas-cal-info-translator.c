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
 * \brief Private function to decode an EasSystemTime struct containing details of a recurring
 *        timezone change pattern, modify it to a format iCalendar expects and create the
 *        required RRULE property value.
 *
 * \param date        Pointer to the EasSystemTime struct. This will be modified in this
 *                    function to contain the first instance of the recurring sequence.
 * \param rrule       Pointer to an icalrecurrencetype struct to store the recurrence rule in.
 */
static void _util_convert_relative_timezone_date(EasSystemTime* date, struct icalrecurrencetype* rrule)
{
	// Microsoft's SYSTEMTIME (implemented here as EasSystemTime) stores a recurring date pattern as follows:
	//   - Year       set to 0
	//   - DayOfWeek  set to the weekday to repeat on (0 = Sunday,...)
	//   - Day        set to the nth weekday to repeat on (1 = first DayOfWeek of the month ... 5 = last of the month)
	//
	// For iCalendar, we have to convert this into the following:
	//   - A DSTART property identifying the first date of the recurring sequence in a year
	//     guaranteed to fall before any events using this timezone pattern. (We're using
	//     1970 as the start of the Unix epoch)
	//   - An RRULE property defining the recurring sequence, in the following format:
	//
	//         FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10
	//
	// In the RRULE, the number in the BYDAY value denotes the nth occurrence in the month,
	// with a negatve value meaning the nth *last* occurrence in the month.
	//
	// So, we have all the information we need in the SYSTEMTIME but there's quite a bit of
	// conversin to do to get to the iCalendar properties.

	EasSystemTime   modifiedDate;
	GDate*          recurrenceStartDate;
	GDateWeekday    weekday;
	gint            occurrence;
	short           byDayPos;
	short           byDayDayOfWeek;

	// We're going to modify the value of date to reflect the iCal values. (e.g. the Year
	// will change from 0 to 1970 and the Day will change from a 1..5 "nth occurrence in the
	// month" value to an actual date of the required month in 1970. we'll use a copy to do
	// themodifications on then assign it back before we return.
	memcpy(&modifiedDate, date, sizeof(EasSystemTime));

	modifiedDate.Year = EPOCH_START_YEAR;

	// Now adjust the DayOfWeek.
	// SYSTEMTIME's wDayOfWeek has 0 = Sunday, 1 = Monday,...     http://msdn.microsoft.com/en-us/library/ms724950(v=vs.85).aspx
	// GDateWeekDay has 0 = G_DATE_BAD_WEEKDAY, 1 = Monday,...    http://developer.gnome.org/glib/stable/glib-Date-and-Time-Functions.html#GDateWeekday
	if (modifiedDate.DayOfWeek == 0)
	{
		modifiedDate.DayOfWeek = 7;
		// Now our DayOfWeek matches GDateWeekDay (see below)
	}

	// date->DayOfWeek will give us the day to repeat on
	// date->Day will give us the nth occurrence in the month of this day
	// (where 5 == last, even if there are only 4)

	// We're going to build a GDate object to calculate the actual date in 1970 that represents
	// the first instance of this recurrence pattern.	
	// Start by initialising to the 1st of of the month
	recurrenceStartDate = g_date_new_dmy(1, modifiedDate.Month, modifiedDate.Year);

	// Now seek for the first occurrence of the day the sequence repeats on.
	weekday = (GDateWeekday)modifiedDate.DayOfWeek;
	while (g_date_get_weekday(recurrenceStartDate) != weekday)
	{
		g_date_add_days(recurrenceStartDate, 1);
	}

	// Now we've got the FIRST occurence of the correct weekday in the correct month in 1970.
	// Finally we need to seek for the nth occurrence (where 5th = last, even if there are only 4)
	occurrence = 1;
	while (occurrence++ < date->Day)
	{
		g_date_add_days(recurrenceStartDate, 7);

		// Check we havn't overrun the end of the month
		// (If we have, just roll back and we're done: we're on the last occurrence)
		if (g_date_get_month(recurrenceStartDate) != modifiedDate.Month)
		{
			g_date_subtract_days(recurrenceStartDate, 7);
			break;
		}
	}

	modifiedDate.Day = g_date_get_day(recurrenceStartDate);

	// Now populate the rrule value before modifying date->Day
	rrule->freq = ICAL_YEARLY_RECURRENCE;
	rrule->by_month[0] = modifiedDate.Month;
	
	// The by_day value in icalrecurrencetype is a short containing two
	// fields: the day, and the position in the month. Weirdly there's no
	// API to encode this (only to decode it) so we'll have to do it by hand.
	// Unfortunaely the comment in icalrecur.c ("The day's position in the
	// period (Nth-ness) and the numerical value of the day are encoded
	// together as: pos*7 + dow") is wrong. :-\ From the code we can see it's
	// actually as implemented below.
	byDayPos = (short)date->Day;
	if (byDayPos == 5)
	{
		byDayPos = -1;
	}
	// Here the day is represented by the enum icalrecurrencetype_weekday, which has the
	// range 0 = ICAL_NO_WEEKDAY, 1 = ICAL_SUNDAY_WEEKDAY, 2 = ICAL_MONDAY_WEEKDAY...
	// so we can just add 1 to the EasSystemTime value (see above).
	byDayDayOfWeek = (short)(date->DayOfWeek + 1);

	// Now calculate the composite value as follows:
	//  - Multiply byDayPos by 8
	//  - Add byDayDayOfWeek if byDayPos is positive, or subtract if negative
	rrule->by_day[0] =
		(byDayPos * 8) +
		((byDayPos >= 0) ? byDayDayOfWeek : (-1 * byDayDayOfWeek));


	// Finally, copy the modified version back into the date that was passed in
	memcpy(date, &modifiedDate, sizeof(EasSystemTime));
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
	// Variable for the return value
	gchar*      result = NULL;

	// Variable for property values as they're read from the XML
	gchar*      value  = NULL;

	// Variable to store the TZID value when decoding a <calendar:Timezone> element
	// so we can use it in the rest of the iCal's date/time fields.
	// TODO: check <calendar:Timezone> always occurs at the top of the calendar item.
	gchar*      tzid         = NULL;

	// iCalendar objects
	struct icaltimetype dateTime;
	struct icaltriggertype trigger;

	if (node && (node->type == XML_ELEMENT_NODE) && (!g_strcmp0((char*)(node->name), "ApplicationData")))
	{
		xmlNodePtr n = node;

		EasCalInfo* cal_info = NULL;

		// iCalendar objects
		icalcomponent* vcalendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);
		icalcomponent* vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);
		icalcomponent* valarm = icalcomponent_new(ICAL_VALARM_COMPONENT);
		icalcomponent* vtimezone = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);
		icalproperty* prop = NULL;
		icalparameter* param = NULL;

		// TODO: get all these strings into constants/#defines
		
		// TODO: make the PRODID configurable somehow
		prop = icalproperty_new_prodid("-//Meego//ActiveSyncD 1.0//EN");
		icalcomponent_add_property(vcalendar, prop);

		prop = icalproperty_new_version("2.0");
		icalcomponent_add_property(vcalendar, prop);

		prop = icalproperty_new_method(ICAL_METHOD_PUBLISH);
		icalcomponent_add_property(vcalendar, prop);

		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);

				if (g_strcmp0(name, "Subject") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_summary(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "StartTime") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					dateTime = icaltime_from_string(value);
					prop = icalproperty_new_dtstart(dateTime);
					if (tzid && strlen(tzid) && !dateTime.is_utc) // Note: TZID not specified if it's a UTC time
					{
						param = icalparameter_new_tzid(tzid);
						icalproperty_add_parameter(prop, param);
					}
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "EndTime") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					dateTime = icaltime_from_string(value);
					prop = icalproperty_new_dtend(dateTime);
					if (tzid && strlen(tzid) && !dateTime.is_utc) // Note: TZID not specified if it's a UTC time
					{
						param = icalparameter_new_tzid(tzid);
						icalproperty_add_parameter(prop, param);
					}
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "DtStamp") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					dateTime = icaltime_from_string(value);
					prop = icalproperty_new_dtstamp(dateTime);
					if (tzid && strlen(tzid) && !dateTime.is_utc) // Note: TZID not specified if it's a UTC time
					{
						param = icalparameter_new_tzid(tzid);
						icalproperty_add_parameter(prop, param);
					}
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "UID") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_uid(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "Location") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_location(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "Body") == 0)
				{
					xmlNodePtr subNode = NULL;
					for (subNode = n->children; subNode; subNode = subNode->next)
					{
						if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar*)subNode->name, "Data"))
						{
							value = (gchar*)xmlNodeGetContent(subNode);
							prop = icalproperty_new_description(value);
							icalcomponent_add_property(vevent, prop);
							g_free(value); value = NULL;
							break;
						}
					}
				}
				else if (g_strcmp0(name, "Sensitivity") == 0)
				{
					icalproperty_class classValue = ICAL_CLASS_PUBLIC;
					value = (gchar*)xmlNodeGetContent(n);
					if (g_strcmp0(value, "3") == 0)      // Confidential
					{
						classValue = ICAL_CLASS_CONFIDENTIAL;
					}
					else if (g_strcmp0(value, "2") == 0) // Private
					{
						classValue = ICAL_CLASS_PRIVATE;
					}
					else // Personal or Normal (iCal doesn't distinguish between them)
					{
						classValue = ICAL_CLASS_PUBLIC;
					}
					prop = icalproperty_new_class(classValue);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}
				else if (g_strcmp0(name, "BusyStatus") == 0)
				{
					icalproperty_transp transpValue = ICAL_TRANSP_OPAQUE;
					value = (gchar*)xmlNodeGetContent(n);
					if (g_strcmp0(value, "0") == 0) // Free
					{
						transpValue = ICAL_TRANSP_TRANSPARENT;
					}
					else // Tentative, Busy or Out of Office
					{
						transpValue = ICAL_TRANSP_OPAQUE;
					}
					prop = icalproperty_new_transp(transpValue);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
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

						value = (gchar*)xmlNodeGetContent(catNode);
						categories = g_string_append(categories, value);
						g_free(value); value = NULL;
					}
					if (categories->len > 0)
					{
						prop = icalproperty_new_categories(categories->str);
						icalcomponent_add_property(vevent, prop);
					}

					// Free the string, including the character buffer
					g_string_free(categories, TRUE);
				}
				else if (g_strcmp0(name, "Reminder") == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);

					// Build an icaltriggertype structure
					trigger = icaltriggertype_from_int(0); // Null the fields first
					trigger.duration.is_neg = 1;
					trigger.duration.minutes = (unsigned int)g_ascii_strtoll(value, NULL, 10);

					prop = icalproperty_new_action(ICAL_ACTION_DISPLAY);
					icalcomponent_add_property(valarm, prop);

					prop = icalproperty_new_description("Reminder"); // TODO: make this configurable
					icalcomponent_add_property(valarm, prop);

					prop = icalproperty_new_trigger(trigger);
					icalcomponent_add_property(valarm, prop);

					g_free(value); value = NULL;
				}
				else if(g_strcmp0(name, "Attendees") == 0)
				{
					xmlNode* attendeeNode = NULL;

					g_debug("Attendees element found in EAS XML");

					for (attendeeNode = n->children; attendeeNode; attendeeNode = attendeeNode->next)
					{
						// Variables for attendee properties and parameters
						gchar*                   email      = NULL;
						gchar*                   name       = NULL;
						icalparameter_partstat   partstat   = ICAL_PARTSTAT_NONE;	// Participatant Status parameter
						icalparameter_role       role       = ICAL_ROLE_NONE;
						xmlNodePtr               subNode    = NULL;

						g_debug("Attendee element found in EAS XML");

						for (subNode = attendeeNode->children; subNode; subNode = subNode->next)
						{
							if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, "Attendee_Email") == 0)
							{
								email = (gchar*)xmlNodeGetContent(subNode);
							}								
							else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, "Attendee_Name") == 0)
							{
								name = (gchar*)xmlNodeGetContent(subNode);								
							}
							else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, "Attendee_Status") == 0)
							{
								gint64 status = 0;
								value = (gchar*)xmlNodeGetContent(subNode);
								status = g_ascii_strtoll(value, NULL, 10);
								switch(status)
								{
									case 0: // Response unknown
									case 5: // Not responded
										partstat = ICAL_PARTSTAT_NEEDSACTION;
										break;
									case 2: // Tentative
										partstat = ICAL_PARTSTAT_TENTATIVE;
										break;									
									case 3: // Accept
										partstat = ICAL_PARTSTAT_ACCEPTED;
										break;	
									case 4: // Decline
										partstat = ICAL_PARTSTAT_DECLINED;
										break;									
									default:
										partstat = ICAL_PARTSTAT_NONE;
										g_warning("unrecognised attendee status received");
										break;
								}// end switch status
								
								g_free(value); value = NULL;
							}
							else if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar*)subNode->name, "Attendee_Type"))
							{
								gint64 roleValue = 0;
								value = (gchar*)xmlNodeGetContent(subNode);
								roleValue = g_ascii_strtoll(value, NULL, 10);
								switch (roleValue)
								{
									// TODO create an enum for these values
									case 1: //Required
										role = ICAL_ROLE_REQPARTICIPANT;
										break;
									case 2: //Optional
										role = ICAL_ROLE_OPTPARTICIPANT;
										break;
									case 3: //Resource
										role = ICAL_ROLE_NONPARTICIPANT;
										break;
									default:
										role = ICAL_ROLE_NONE;
										g_warning("unrecognised attendee type received");
										break;
								}// end switch type
								
								g_free(value); value = NULL;
							}		
							
						}// end for subNodes

						// Now finally build and add the property, assuming we have at least an e-mail address
						if (email && strlen(email))
						{
							prop = icalproperty_new_attendee(email);

							if (email && strlen(name))
							{
								param = icalparameter_new_cn(name);
								icalproperty_add_parameter(prop, param);
							}
							if (partstat != ICAL_PARTSTAT_NONE)
							{
								param = icalparameter_new_partstat(partstat);
								icalproperty_add_parameter(prop, param);
							}
							if (role != ICAL_ROLE_NONE)
							{
								param = icalparameter_new_role(role);
								icalproperty_add_parameter(prop, param);
							}
							
							icalcomponent_add_property(vevent, prop);
						}
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
						memcpy(&timeZoneStruct, timeZoneRawBytes, timeZoneRawBytesSize);
						g_free(timeZoneRawBytes);
						timeZoneRawBytes = NULL;

						{
							// Calculate the timezone offsets. See _util_process_vtimezone_subcomponent()
							// comments for a full explanation of how EAS Bias relates to iCal UTC offsets
							const gint32                standardUtcOffsetMins = -1 * timeZoneStruct.Bias;
							const gint32                daylightUtcOffsetMins = -1 * (timeZoneStruct.Bias + timeZoneStruct.DaylightBias);
							icalcomponent*              xstandard             = NULL;
							icalcomponent*              xdaylight             = NULL;
							struct icalrecurrencetype   rrule;
							struct icaltimetype         time;
							
						
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
							prop = icalproperty_new_tzid(tzid);
							icalcomponent_add_property(vtimezone, prop);

						
							// STANDARD component
							xstandard = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);

							icalrecurrencetype_clear(&rrule);

							// If timeZoneStruct.StandardDate.Year == 0 we need to convert it into
							// the start date of a recurring sequence, and add an RRULE.
							if (timeZoneStruct.StandardDate.Year == 0)
							{
								_util_convert_relative_timezone_date(&timeZoneStruct.StandardDate, &rrule);
							}

							// Add the DTSTART property
							time = icaltime_null_time();
							time.year = timeZoneStruct.StandardDate.Year;
							time.month = timeZoneStruct.StandardDate.Month;
							time.day = timeZoneStruct.StandardDate.Day;
							time.hour = timeZoneStruct.StandardDate.Hour;
							time.minute = timeZoneStruct.StandardDate.Minute;
							time.second = timeZoneStruct.StandardDate.Second;
							prop = icalproperty_new_dtstart(time);
							icalcomponent_add_property(xstandard, prop);

							// Add the RRULE (if required)
							if (rrule.freq != ICAL_NO_RECURRENCE)
							{
								prop = icalproperty_new_rrule(rrule);
								icalcomponent_add_property(xstandard, prop);
							}

							// Add TZOFFSETFROM and TZOFFSETTO
							// Note that libical expects these properties in seconds.
							prop = icalproperty_new_tzoffsetfrom(daylightUtcOffsetMins * SECONDS_PER_MINUTE);
							icalcomponent_add_property(xstandard, prop);
							prop = icalproperty_new_tzoffsetto(standardUtcOffsetMins * SECONDS_PER_MINUTE);
							icalcomponent_add_property(xstandard, prop);

							value = g_utf16_to_utf8((const gunichar2*)timeZoneStruct.StandardName, (sizeof(timeZoneStruct.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
							if (value)
							{
								if (strlen(value))
								{
									prop = icalproperty_new_tzname(value);
									icalcomponent_add_property(xstandard, prop);
								}
								g_free(value); value = NULL;
							}

							// And now add the STANDARD component to the VTIMEZONE
							icalcomponent_add_component(vtimezone, xstandard);

						
							// DAYLIGHT component
						
							xdaylight = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);

							// Reset the RRULE
							icalrecurrencetype_clear(&rrule);

							// If timeZoneStruct.DaylightDate.Year == 0 we need to convert it into
							// the start date of a recurring sequence, and add an RRULE.
							if (timeZoneStruct.DaylightDate.Year == 0)
							{
								_util_convert_relative_timezone_date(&timeZoneStruct.DaylightDate, &rrule);
							}

							// Add the DTSTART property
							time = icaltime_null_time();
							time.year = timeZoneStruct.DaylightDate.Year;
							time.month = timeZoneStruct.DaylightDate.Month;
							time.day = timeZoneStruct.DaylightDate.Day;
							time.hour = timeZoneStruct.DaylightDate.Hour;
							time.minute = timeZoneStruct.DaylightDate.Minute;
							time.second = timeZoneStruct.DaylightDate.Second;
							prop = icalproperty_new_dtstart(time);
							icalcomponent_add_property(xdaylight, prop);

							// Add the RRULE (if required)
							if (rrule.freq != ICAL_NO_RECURRENCE)
							{
								prop = icalproperty_new_rrule(rrule);
								icalcomponent_add_property(xdaylight, prop);
							}

							// Add TZOFFSETFROM and TZOFFSETTO
							// Note that libical expects these properties in seconds.
							prop = icalproperty_new_tzoffsetfrom(standardUtcOffsetMins * SECONDS_PER_MINUTE);
							icalcomponent_add_property(xdaylight, prop);
							prop = icalproperty_new_tzoffsetto(daylightUtcOffsetMins * SECONDS_PER_MINUTE);
							icalcomponent_add_property(xdaylight, prop);

							value = g_utf16_to_utf8((const gunichar2*)timeZoneStruct.DaylightName, (sizeof(timeZoneStruct.DaylightName)/sizeof(guint16)), NULL, NULL, NULL);
							if (value)
							{
								if (strlen(value))
								{
									prop = icalproperty_new_tzname(value);
									icalcomponent_add_property(xdaylight, prop);
								}
								g_free(value); value = NULL;
							}

							// And now add the DAYLIGHT component to the VTIMEZONE
							icalcomponent_add_component(vtimezone, xdaylight);
						}
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


		// Add the subcomponens to their parent components
        if (icalcomponent_count_properties(vtimezone, ICAL_ANY_PROPERTY) > 0)
        {
			icalcomponent_add_component(vcalendar, vtimezone);
		}
		icalcomponent_add_component(vcalendar, vevent);
        if (icalcomponent_count_properties(valarm, ICAL_ANY_PROPERTY) > 0)
        {
			icalcomponent_add_component(vevent, valarm);
		}

		// Now insert the server ID and iCalendar into an EasCalInfo object and serialise it
		cal_info = eas_cal_info_new();
		cal_info->icalendar = (gchar*)icalcomponent_as_ical_string_r(vcalendar); // Ownership passes to the EasCalInfo
		cal_info->server_id = (gchar*)server_id;
		if (!eas_cal_info_serialise(cal_info, &result))
		{
			// TODO: log error
			result = NULL;
		}

		// Free the EasCalInfo GObject
		g_object_unref(cal_info);

		// Free the libical components
		// (It's not clear if freeing a component also frees its children, but in any case
		// some of these (e.g. vtimezone & valarm) won't have been added as children if they
		// weren't present in the XML.)
		icalcomponent_free(valarm);
		icalcomponent_free(vevent);
		icalcomponent_free(vtimezone);
		icalcomponent_free(vcalendar);
		// Note: the libical examples show that a property doesn't have to be freed once added to a component

		// Free the TZID string
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
				case ICAL_NO_WEEKDAY:
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
		icalcomponent* subcomponent = NULL;
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
		icalcomponent* vevent = NULL;
		
		// Process the components of the VCALENDAR
		_util_process_vtimezone_component(icalcomponent_get_first_component(ical, ICAL_VTIMEZONE_COMPONENT), app_data);
		vevent = icalcomponent_get_first_component(ical, ICAL_VEVENT_COMPONENT);
		_util_process_vevent_component(vevent, app_data);
		_util_process_valarm_component(icalcomponent_get_first_component(vevent, ICAL_VALARM_COMPONENT), app_data);

		// DEBUG output
		{
			xmlChar* dump_buffer;
			int dump_buffer_size;
			xmlIndentTreeOutput = 1;
			xmlDocDumpFormatMemory(doc, &dump_buffer, &dump_buffer_size, 1);
			g_debug("XML DOCUMENT DUMPED:\n%s", dump_buffer);
			xmlFree(dump_buffer);
		}
		
		success = TRUE;
	}

	if (ical)
	{
		icalcomponent_free(ical);
	}

	return success;
}

