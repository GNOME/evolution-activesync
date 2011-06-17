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

// Values for converting icaldurationtype into a number of minutes
const gint SECONDS_PER_MINUTE          = 60;
const gint MINUTES_PER_HOUR            = 60;
const gint MINUTES_PER_DAY             = 60 * 24;
const gint SECONDS_PER_DAY             = 60 * 60 * 24;
const gint MINUTES_PER_WEEK            = 60 * 24 * 7;
const gint DAYS_PER_WEEK               = 7;
const gint EPOCH_START_YEAR            = 1970;

// Constants for <Calendar> parsing
const guint DAY_OF_WEEK_SUNDAY         = 0x00000001;
const guint DAY_OF_WEEK_MONDAY         = 0x00000002;
const guint DAY_OF_WEEK_TUESDAY        = 0x00000004;
const guint DAY_OF_WEEK_WEDNESDAY      = 0x00000008;
const guint DAY_OF_WEEK_THURSDAY       = 0x00000010;
const guint DAY_OF_WEEK_FRIDAY         = 0x00000020;
const guint DAY_OF_WEEK_SATURDAY       = 0x00000040;
const guint DAY_OF_WEEK_LAST_OF_MONTH  = 0x0000007F; // 127



// EAS string value definitions
#define EAS_NAMESPACE_CALENDAR                "calendar:"
#define EAS_NAMESPACE_AIRSYNCBASE             "airsyncbase:"
#define X_NAMESPACE_ACTIVESYNCD               "activesyncd:"
#define X_ELEMENT_ICALEXENSIONS               "iCalExtensions"
#define EAS_ELEMENT_APPLICATIONDATA           "ApplicationData"
#define EAS_ELEMENT_TIMEZONE                  "TimeZone"
#define EAS_ELEMENT_ALLDAYEVENT               "AllDayEvent"
#define EAS_ELEMENT_BUSYSTATUS                "BusyStatus"
#define EAS_ELEMENT_ORGANIZER_NAME            "Organizer_Name"
#define EAS_ELEMENT_ORGANIZER_EMAIL           "Organizer_Email"
#define EAS_ELEMENT_DTSTAMP                   "DtStamp"
#define EAS_ELEMENT_ENDTIME                   "EndTime"
#define EAS_ELEMENT_LOCATION                  "Location"
#define EAS_ELEMENT_REMINDER                  "Reminder"
#define EAS_ELEMENT_SENSITIVITY               "Sensitivity"
#define EAS_ELEMENT_SUBJECT                   "Subject"
#define EAS_ELEMENT_STARTTIME                 "StartTime"
#define EAS_ELEMENT_UID                       "UID"
#define EAS_ELEMENT_MEETINGSTATUS             "MeetingStatus"
#define EAS_ELEMENT_ATTENDEES                 "Attendees"
#define EAS_ELEMENT_ATTENDEE                  "Attendee"
#define EAS_ELEMENT_ATTENDEE_EMAIL            "Attendee_Email"
#define EAS_ELEMENT_ATTENDEE_NAME             "Attendee_Name"
#define EAS_ELEMENT_ATTENDEE_STATUS           "Attendee_Status"
#define EAS_ELEMENT_ATTENDEE_TYPE             "Attendee_Type"
#define EAS_ELEMENT_CATEGORIES                "Categories"
#define EAS_ELEMENT_CATEGORY                  "Category"
#define EAS_ELEMENT_RECURRENCE                "Recurrence"
#define EAS_ELEMENT_TYPE                      "Type"
#define EAS_ELEMENT_OCCURRENCES               "Occurrences"
#define EAS_ELEMENT_INTERVAL                  "Interval"
#define EAS_ELEMENT_WEEKOFMONTH               "WeekOfMonth"
#define EAS_ELEMENT_DAYOFWEEK                 "DayOfWeek"
#define EAS_ELEMENT_MONTHOFYEAR               "MonthOfYear"
#define EAS_ELEMENT_UNTIL                     "Until"
#define EAS_ELEMENT_DAYOFMONTH                "DayOfMonth"
#define EAS_ELEMENT_CALENDARTYPE              "CalendarType"
#define EAS_ELEMENT_ISLEAPMONTH               "IsLeapMonth"
#define EAS_ELEMENT_FIRSTDAYOFWEEK            "FirstDayOfWeek"
#define EAS_ELEMENT_EXCEPTIONS                "Exceptions"
#define EAS_ELEMENT_EXCEPTION                 "Exception"
#define EAS_ELEMENT_DELETED                   "Deleted"
#define EAS_ELEMENT_EXCEPTIONSTARTTIME        "ExceptionStartTime"
#define EAS_ELEMENT_APPOINTMENTREPLYTIME      "AppointmentReplyTime"
#define EAS_ELEMENT_RESPONSETYPE              "ResponseType"
#define EAS_ELEMENT_BODY                      "Body"
#define EAS_ELEMENT_RESPONSEREQUESTED         "ResponseRequested"
#define EAS_ELEMENT_NATIVEBODYTYPE            "NativeBodyType"
#define EAS_ELEMENT_ONLINEMEETINGCONFLINK     "OnlineMeetingConfLink"
#define EAS_ELEMENT_ONLINEMEETINGEXTERNALLINK "OnlineMeetingExternalLink"
#define EAS_ELEMENT_DATA                      "Data"

#define EAS_SENSITIVITY_NORMAL                "0"
#define EAS_SENSITIVITY_PERSONAL              "1"
#define EAS_SENSITIVITY_PRIVATE               "2"
#define EAS_SENSITIVITY_CONFIDENTIAL          "3"

#define EAS_BUSYSTATUS_FREE                   "0"
#define EAS_BUSYSTATUS_TENTATIVE              "1"
#define EAS_BUSYSTATUS_BUSY                   "2"
#define EAS_BUSYSTATUS_OUTOFOFFICE            "3"

#define EAS_BOOLEAN_FALSE                     "0"
#define EAS_BOOLEAN_TRUE                      "1"

// Other assorted string constants
#define ICAL_PROPERTY_PRODID                  "-//Meego//ActiveSyncD 1.0//EN"
#define ICAL_PROPERTY_VERSION                 "2.0"
#define ICAL_EXTENSION_PROPERTY_PREFIX        "X-MEEGO-ACTIVESYNCD-"
#define ICAL_DEFAULT_TZID                     "Standard Timezone" // Only used internally, not visible to user
#define ICAL_DEFAULT_REMINDER_NAME            "Reminder"





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
 * \brief Convert a <Sensitivity> value from EAS XML into an iCal CLASS property value
 */
static icalproperty_class _eas2ical_convert_sensitivity_to_class(const gchar* sensitivityValue)
{
	if (g_strcmp0(sensitivityValue, EAS_SENSITIVITY_CONFIDENTIAL) == 0)
	{
		return ICAL_CLASS_CONFIDENTIAL;
	}
	else if (g_strcmp0(sensitivityValue, EAS_SENSITIVITY_PRIVATE) == 0)
	{
		return ICAL_CLASS_PRIVATE;
	}
	else // Personal or Normal (iCal doesn't distinguish between them)
	{
		return ICAL_CLASS_PUBLIC;
	}
}


/**
 * \brief Convert a <BusyStatus> value from EAS XML into an iCal TRANSP property value
 */
static icalproperty_transp _eas2ical_convert_busystatus_to_transp(const gchar* busystatusValue)
{
	if (g_strcmp0(busystatusValue, EAS_BUSYSTATUS_FREE) == 0)
	{
		return ICAL_TRANSP_TRANSPARENT;
	}
	else // Tentative, Busy or Out of Office
	{
		return ICAL_TRANSP_OPAQUE;
	}
}


/**
 * \brief The by_day member of icalrecurrencetype is a composite value, encoding
 *        both the day to repeat on and (in the case of monthly recurrence) the position
 *        of the day to repeat on (e.g. 2nd Monday in each month). libical doesn't provide
 *        an API to encode this, so we've had to write our own by reverse engineering
 *        the implementation of icalrecurrencetype_day_day_of_week() and
 *        icalrecurrencetype_day_position() in icalrecur.c.
 *
 * \param byDayDayOfWeek  The day of the week to repeat on
 * \param byDayPos        The position of this day in the month on which to repeat
 *                        (or 0 if not applicable)
 */
static short _eas2ical_make_rrule_by_day_value(icalrecurrencetype_weekday byDayDayOfWeek, short byDayPos)
{
	// Now calculate the composite value as follows:
	//  - Multiply byDayPos by 8
	//  - Add byDayDayOfWeek if byDayPos is positive, or subtract if negative
	short dowShort = (short)byDayDayOfWeek;
	return
		(byDayPos * 8) +
		((byDayPos >= 0) ? dowShort : (-1 * dowShort));
}


/**
 * \brief Private function to decode an EasSystemTime struct containing details of a recurring
 *        timezone change pattern, modify it to a format iCalendar expects and create the
 *        required RRULE property value.
 *
 * \param date        Pointer to the EasSystemTime struct. This will be modified in this
 *                    function to contain the first instance of the recurring sequence.
 * \param rrule       Pointer to an icalrecurrencetype struct to store the recurrence rule in.
 */
static void _eas2ical_convert_relative_timezone_date(EasSystemTime* date, struct icalrecurrencetype* rrule)
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

	EasSystemTime              modifiedDate;
	GDate*                     recurrenceStartDate;
	GDateWeekday               weekday;
	gint                       occurrence;
	short                      byDayPos;
	icalrecurrencetype_weekday byDayDayOfWeek;

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
		modifiedDate.DayOfWeek = (guint16)G_DATE_SUNDAY;
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
		g_date_add_days(recurrenceStartDate, DAYS_PER_WEEK);

		// Check we havn't overrun the end of the month
		// (If we have, just roll back and we're done: we're on the last occurrence)
		if (g_date_get_month(recurrenceStartDate) != modifiedDate.Month)
		{
			g_date_subtract_days(recurrenceStartDate, DAYS_PER_WEEK);
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
	// Unfortunately the comment in icalrecur.c ("The day's position in the
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
	byDayDayOfWeek = (icalrecurrencetype_weekday)(date->DayOfWeek + 1);

	// Now calculate the composite value as follows:
	//  - Multiply byDayPos by 8
	//  - Add byDayDayOfWeek if byDayPos is positive, or subtract if negative
	rrule->by_day[0] = _eas2ical_make_rrule_by_day_value(byDayDayOfWeek, byDayPos);

	// Finally, copy the modified version back into the date that was passed in
	memcpy(date, &modifiedDate, sizeof(EasSystemTime));
}


/**
 * \brief Process the <Attendees> element during parsing of an EAS XML document
 *
 * \param n       An XML node pointing at the <Attendees> element
 * \param vevent  The VEVENT iCal component to add the parsed attendees to
 */
static void _eas2ical_process_attendees(xmlNodePtr n, icalcomponent* vevent)
{
	gchar*         value = NULL;
	icalproperty*  prop  = NULL;
	icalparameter* param = NULL;

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
			if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, EAS_ELEMENT_ATTENDEE_EMAIL) == 0)
			{
				email = (gchar*)xmlNodeGetContent(subNode);
			}								
			else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, EAS_ELEMENT_ATTENDEE_NAME) == 0)
			{
				name = (gchar*)xmlNodeGetContent(subNode);								
			}
			else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0((gchar*)subNode->name, EAS_ELEMENT_ATTENDEE_STATUS) == 0)
			{
				int status = 0;
				value = (gchar*)xmlNodeGetContent(subNode);
				status = atoi(value);
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
			else if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar*)subNode->name, EAS_ELEMENT_ATTENDEE_TYPE))
			{
				int roleValue = 0;
				value = (gchar*)xmlNodeGetContent(subNode);
				roleValue = atoi(value);
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
	}
}


/**
 * \brief Process the <Timezone> element during parsing of an EAS XML document
 *
 * \param n          An XML node pointing to the <Recurrence> element
 * \param vtimezone  The iCalendar VTIMEZONE component to add the parsed timezone properties to
 * \param tzid       Pointer to a buffer to initialise with the parsed TZID (timezone ID) string
 */
static void _eas2ical_process_timezone(xmlNodePtr n, icalcomponent* vtimezone, gchar** tzid)
{
	gchar*         value = NULL;
	icalproperty*  prop = NULL;
	xmlChar*       timeZoneBase64Buffer = xmlNodeGetContent(n);
	gsize          timeZoneRawBytesSize = 0;
	guchar*        timeZoneRawBytes = g_base64_decode((const gchar*)timeZoneBase64Buffer, &timeZoneRawBytesSize);
	EasTimeZone    timeZoneStruct;
	
	xmlFree(timeZoneBase64Buffer);

	// TODO Check decode of timezone for endianess problems

	if (timeZoneRawBytesSize == sizeof(EasTimeZone))
	{
		memcpy(&timeZoneStruct, timeZoneRawBytes, timeZoneRawBytesSize);
		g_free(timeZoneRawBytes);
		timeZoneRawBytes = NULL;

		{
			// Calculate the timezone offsets. See _ical2eas_process_xstandard_xdaylight()
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
			*tzid = g_utf16_to_utf8((const gunichar2*)timeZoneStruct.StandardName,
				                   (sizeof(timeZoneStruct.StandardName)/sizeof(guint16)), NULL, NULL, NULL);
			// If no StandardName was supplied, we can just use a temporary name instead.
			// No need to support more than one: the EAS calendar will only have one Timezone
			// element. And no need to localise, as it's only used internally.
			if (*tzid == NULL || strlen(*tzid) == 0)
			{
				*tzid = g_strdup(ICAL_DEFAULT_TZID);
			}
			prop = icalproperty_new_tzid(*tzid);
			icalcomponent_add_property(vtimezone, prop);

			
			//
			// STANDARD component
			//
			
			xstandard = icalcomponent_new(ICAL_XSTANDARD_COMPONENT);

			icalrecurrencetype_clear(&rrule);

			// If timeZoneStruct.StandardDate.Year == 0 we need to convert it into
			// the start date of a recurring sequence, and add an RRULE.
			if (timeZoneStruct.StandardDate.Year == 0)
			{
				_eas2ical_convert_relative_timezone_date(&timeZoneStruct.StandardDate, &rrule);
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

			
			//
			// DAYLIGHT component
			//
		
			xdaylight = icalcomponent_new(ICAL_XDAYLIGHT_COMPONENT);

			// Reset the RRULE
			icalrecurrencetype_clear(&rrule);

			// If timeZoneStruct.DaylightDate.Year == 0 we need to convert it into
			// the start date of a recurring sequence, and add an RRULE.
			if (timeZoneStruct.DaylightDate.Year == 0)
			{
				_eas2ical_convert_relative_timezone_date(&timeZoneStruct.DaylightDate, &rrule);
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


/**
 * \brief Process the <Recurrence> element during parsing of an EAS XML document
 *
 * \param n       An XML node pointing to the <Recurrence> element
 * \param vevent  The iCalendar VEVENT component to add the parsed RRULE property to
 */
static void _eas2ical_process_recurrence(xmlNodePtr n, icalcomponent* vevent)
{
	gchar*         value    = NULL;
	icalproperty*  prop     = NULL;
	xmlNode*       subNode  = NULL;
	struct icalrecurrencetype recur;

	// Ensure the icalrecurrencetype is null
	icalrecurrencetype_clear(&recur);

	g_debug("Recurrence element found in EAS XML");

	for (subNode = n->children; subNode; subNode = subNode->next)
	{
		const gchar* elemName = (const gchar*)subNode->name;
		value = (gchar*)xmlNodeGetContent(subNode);

		// Type
		if (g_strcmp0(elemName, EAS_ELEMENT_TYPE) == 0)
		{
			int typeInt = atoi(value);
			switch (typeInt)
			{
				case 0: // Recurs daily
					recur.freq = ICAL_DAILY_RECURRENCE;
					break;
				case 1: // Recurs weekly
					recur.freq = ICAL_WEEKLY_RECURRENCE;
					break;
				case 2: // Recurs monthly
					recur.freq = ICAL_MONTHLY_RECURRENCE;
					break;
				case 3: // Recurs monthly on the nth day
					recur.freq = ICAL_MONTHLY_RECURRENCE;
					break;
				case 5: // Recurs yearly
					recur.freq = ICAL_YEARLY_RECURRENCE;
					break;
				case 6: // Recurs yearly on the nth day
					recur.freq = ICAL_YEARLY_RECURRENCE;
					break;
			}
		}
		
		// Occurrences
		else if (g_strcmp0(elemName, EAS_ELEMENT_OCCURRENCES) == 0)
		{
			// From [MS-ASCAL]:
			// The Occurrences element and the Until element (section 2.2.2.18.7) are mutually exclusive. It is
			// recommended that only one of these elements be included in a Recurrence element (section
			// 2.2.2.18) in a Sync command request. If both elements are included, then the server MUST respect
			// the value of the Occurrences element and ignore the Until element.
			recur.count = atoi((char*)xmlNodeGetContent(subNode));
			recur.until = icaltime_null_time();
		}
		
		// Interval
		else if (g_strcmp0(elemName, EAS_ELEMENT_INTERVAL) == 0)
		{
			recur.interval = (short)atoi((char*)xmlNodeGetContent(subNode));
		}

		// WeekOfMonth
		else if (g_strcmp0(elemName, EAS_ELEMENT_WEEKOFMONTH) == 0)
		{
			g_warning("DATA LOSS: Cannot handle <Calendar><Recurrence><WeekOfMonth> element (value=%d)", atoi((char*)xmlNodeGetContent(subNode)));
		}

		// DayOfWeek
		else if (g_strcmp0(elemName, EAS_ELEMENT_DAYOFWEEK) == 0)
		{
			// A recurrence rule can target any combination of the 7 week days.
			// EAS encodes these as a bit set in a single int value.
			// libical encodes them as an array (max size 7) of icalrecurrencetype_weekday
			// enum values.
			// This block of code converts the former into the latter.

			int dayOfWeek = atoi((char*)xmlNodeGetContent(subNode));
			int index = 0;

			// Note: must use IF for subsequent blocks, not ELSE IF
			if (dayOfWeek & DAY_OF_WEEK_MONDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_MONDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_TUESDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_TUESDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_WEDNESDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_WEDNESDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_THURSDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_THURSDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_FRIDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_FRIDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_SATURDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_SATURDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_SUNDAY)
			{
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value(ICAL_SUNDAY_WEEKDAY, 0);
			}
		}

		// MonthOfYear
		else if (g_strcmp0(elemName, EAS_ELEMENT_MONTHOFYEAR) == 0)
		{
			recur.by_month[0] = (short)atoi((char*)xmlNodeGetContent(subNode));
		}

		// Until
		else if (g_strcmp0(elemName, EAS_ELEMENT_UNTIL) == 0)
		{
			// From [MS-ASCAL]:
			// The Occurrences element and the Until element (section 2.2.2.18.7) are mutually exclusive. It is
			// recommended that only one of these elements be included in a Recurrence element (section
			// 2.2.2.18) in a Sync command request. If both elements are included, then the server MUST respect
			// the value of the Occurrences element and ignore the Until element.
			if (recur.count == 0)
			{
				recur.until = icaltime_from_string((gchar*)xmlNodeGetContent(subNode));
			}
		}

		// DayOfMonth
		else if (g_strcmp0(elemName, EAS_ELEMENT_DAYOFMONTH) == 0)
		{
			recur.by_month_day[0] = (short)atoi((char*)xmlNodeGetContent(subNode));
		}

		// CalendarType
		else if (g_strcmp0(elemName, EAS_ELEMENT_CALENDARTYPE) == 0)
		{
			const int calType = atoi((char*)xmlNodeGetContent(subNode));
			if ((calType != 0)    // Default
			    &&
			    (calType != 1))   // Gregorian
			{
				// No way of handling in iCal
				g_warning("DATA LOSS: Encountered a calendar type we can't handle in the <Calendar><Recurrence><CalendarType> element: %d", calType);
			}
		}

		// IsLeapMonth
		else if (g_strcmp0(elemName, EAS_ELEMENT_ISLEAPMONTH) == 0)
		{
			if (atoi((char*)xmlNodeGetContent(subNode)) == 1)
			{
				// This has nothing to do with Gregorian calendar leap years (ie. 29 days in Feb).
				// It only applies to non-Gregorian calendars so can't be handled in iCal.
				g_warning("DATA LOSS: Cannot handle <Calendar><Recurrence><IsLeapMonth> element");
			}
		}

		// FirstDayOfWeek
		else if (g_strcmp0(elemName, EAS_ELEMENT_FIRSTDAYOFWEEK) == 0)
		{
			int firstDayOfWeek = atoi((char*)xmlNodeGetContent(subNode));

			// EAS value is in range 0=Sunday..6=Saturday
			// iCal value is in range 0=NoWeekday, 1=Sunday..7=Saturday
			recur.week_start = (icalrecurrencetype_weekday)firstDayOfWeek + 1;
		}

		// Other fields...
		else
		{
			g_warning("DATA LOSS: Unknown element encountered in <Recurrence> element: %s", elemName);
		}
	}

	prop = icalproperty_new_rrule(recur);
	icalcomponent_add_property(vevent, prop);
}


/**
 * \brief Process the <Exceptions> element during parsing of an EAS XML document
 *
 * \param n       An XML node pointing to the <Exceptions> element
 * \param vevent  The iCalendar VEVENT component to add the parsed items property to
 */
static GSList* _eas2ical_process_exceptions(xmlNodePtr n, icalcomponent* vevent)
{
	// This is a bit tricky.
	//
	//  - If the EAS <Exception> element ONLY contains <ExceptionStartTime> and
	//    <Deleted> (with the latter's value set to 1), it's a simple exception
	//    that just deletes one of the occurrences of the recurrence (ie. an
	//    EXDATE in iCal);
	//
	//  - If it just contains an <ExceptionStartTime> (and optionally a <Deleted>
	//    element with the value 0), it represents an *additional* occurrence of
	//    the event, with all other attributes identical to those of the recurring
	//    event (ie. an RDATE in iCal);
	//
	//  - However, if it contains additional elements, it represents an exception
	//    where properties have been changed (eg. where the start time or subject
	//    have been changed for one occurrence only). These aren't supported in
	//    iCal so we have to create a whole new event. We can't do that here (as
	//    we're still part-way through parsing the event) so we return a list of
	//    exception event details so they can be processed later. Each exception
	//    event we return is stored as a hash table of property names/values.
	//    (So this function returns a list of hash tables.)

	
	gchar*         value = NULL;
	xmlNode*       exceptionNode = NULL;
	xmlNode*       subNode = NULL;
	GSList*        listOfNewEventEvents = NULL;

	g_debug("Exceptions element found in EAS XML");

	// Iterate through the <Exception> elements
	for (exceptionNode = n->children; exceptionNode; exceptionNode = exceptionNode->next)
	{
		GHashTable* newEventValues = NULL;
		
		// Iterate through each Exception's properties
		gchar* exceptionStartTime = NULL;
		gboolean deleted = FALSE;
		for (subNode = exceptionNode->children; subNode; subNode = subNode->next)
		{
			const gchar* name = (const gchar*)subNode->name;
			value = (gchar*)xmlNodeGetContent(subNode);

			if (g_strcmp0(name, EAS_ELEMENT_DELETED) == 0)
			{
				deleted = (g_strcmp0(value, EAS_BOOLEAN_TRUE) == 0);
			}
			else if (g_strcmp0(name, EAS_ELEMENT_EXCEPTIONSTARTTIME) == 0)
			{
				exceptionStartTime = g_strdup(value);
			}
			else if (strlen(value) > 0)
			{
				// We've got a non-trivial exception that will
				// require adding a new event: build a hash of its values
				// and add to the list
				if (newEventValues == NULL)
				{
					newEventValues = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
				}
				g_hash_table_insert(newEventValues, g_strdup(name), g_strdup(value));
			}

			g_free(value);
		}

		// ExceptionStartTime is mandatory so check we've got one
		if (exceptionStartTime == NULL)
		{
			g_warning("DATA LOSS: <Exception> element found with no ExceptionStartTime.");
			break;
		}

		// If the <Deleted> value is set to 1, it's dead easy: it maps straight onto an
		// EXDATE an we can ignore the other elements.
		if (deleted)
		{
			// Add an EXDATE property

			// I'm ASSUMING here that we add multiple single-value EXDATE properties and
			// libical takes care of merging them into one (just as it splits them when
			// we're reading an iCal). TODO: check this during testing...
			icalproperty* exdate = icalproperty_new_exdate(icaltime_from_string(exceptionStartTime));
			icalcomponent_add_property(vevent, exdate); // vevent takes ownership of exdate
		}
		// If it's not deleted, but the only other element present is ExceptionStartTime,
		// then it's an RDATE (i.e. just a one-off recurrence of the same event but not
		// included in the regular recurrence sequence)
		else if (newEventValues == NULL)
		{
			icalproperty* rdate = NULL;
			
			// Same assumption as for EXDATE: that we just add multiple properties
			// and libical takes care of merging them. TODO: check this during testing...
			struct icaldatetimeperiodtype dtper;
			dtper.period = icalperiodtype_null_period();
			dtper.time = icaltime_from_string(exceptionStartTime);
			rdate = icalproperty_new_rdate(dtper);
			icalcomponent_add_property(vevent, rdate); // vevent takes ownership of rdate
		}
		// Otherwise it's neither an EXDATE or an RDATE: it's a new instance of the
		// event with more substantial changes (e.g. start time/end time/subject/etc.
		// has changed)
		else
		{
			// Add ExceptionStartTime to the hash now too
			g_hash_table_insert(newEventValues, g_strdup(EAS_ELEMENT_EXCEPTIONSTARTTIME), g_strdup(exceptionStartTime));

			// And add the hash table to the list to be returned
			// (Using prepend rather than append for efficiency)
			// (No need to new the list first, it's allocated during prepend:
			// http://developer.gnome.org/glib/stable/glib-Singly-Linked-Lists.html#g-slist-alloc)
			listOfNewEventEvents = g_slist_prepend(listOfNewEventEvents, newEventValues);
		}
		
		g_free(exceptionStartTime);
	}// end of for loop

	return listOfNewEventEvents;
}


/**
 * TODO...
 */
static void _eas2ical_add_exception_events(icalcomponent* vcalendar, icalcomponent* vevent, GSList* exceptionEvents)
{
	if (vcalendar && vevent && exceptionEvents)
	{
		const guint newEventCount = g_slist_length(exceptionEvents);
		guint index = 0;

		// Iterate through the list adding each exception event in turn
		for (index = 0; index < newEventCount; index++)
		{
			icalcomponent* newEvent = icalcomponent_new_clone(vevent);
			GHashTable* exceptionProperties = (GHashTable*)g_slist_nth_data(exceptionEvents, index);
			icalproperty* prop = NULL;
			gchar* value = NULL;

			if (exceptionProperties == NULL)
			{
				g_warning("_eas2ical_add_exception_events(): NULL hash table found in exceptionEvents.");
				break;
			}

			// Remove any recurrence (RRULE, RDATE and EXDATE) properies from the new event
			while ((prop = icalcomponent_get_first_property(newEvent, ICAL_RRULE_PROPERTY)) != NULL)
			{
				icalcomponent_remove_property(newEvent, prop);
				icalproperty_free(prop); prop = NULL;
			}
			while ((prop = icalcomponent_get_first_property(newEvent, ICAL_RDATE_PROPERTY)) != NULL)
			{
				icalcomponent_remove_property(newEvent, prop);
				icalproperty_free(prop); prop = NULL;
			}
			while ((prop = icalcomponent_get_first_property(newEvent, ICAL_EXDATE_PROPERTY)) != NULL)
			{
				icalcomponent_remove_property(newEvent, prop);
				icalproperty_free(prop); prop = NULL;
			}

			// Form a new UID for the new event as follows:
			// {Original event UID}_{Exception start time}
			prop = icalcomponent_get_first_property(newEvent, ICAL_UID_PROPERTY); // Retains ownership of the pointer
			value = g_strconcat((const gchar*)icalproperty_get_uid(prop), "_", (const gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_EXCEPTIONSTARTTIME), NULL);
			icalproperty_set_uid(prop, value);
			prop = NULL;

			// TODO: as we're parsing these from the <Exceptions> element, I think we need to
			// add an EXDATE for this ExceptionStartTime. I think ExceptionStartTime identifies
			// the recurrence this is *replacing*: StartTime specifies this exception's own
			// start time.

			
			// Add the other properties from the hash

			// StartTime
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_STARTTIME)) != NULL)
			{
				// If this exception has a new start time, use that. Otherwise we default
				// to the start time of the original event (as per [MS-ASCAL]).
				// TODO: this needs testing: [MS-ASCAL] is a bit ambiguous around
				// <StartTime> vs. <ExceptionStartTime>
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_DTSTART_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_dtstart(icaltime_from_string(value));
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}

			// EndTime
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_ENDTIME)) != NULL)
			{
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_DTEND_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_dtend(icaltime_from_string(value));
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}

			// Subject
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_SUBJECT)) != NULL)
			{
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_SUMMARY_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_summary(value);
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}

			// Location
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_LOCATION)) != NULL)
			{
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_LOCATION_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_location(value);
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}

			// Categories
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_CATEGORIES)) != NULL)
			{
				// TODO: sort out <Categories> - it's a collection element! :-(
			}

			// Sensitivity
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_SENSITIVITY)) != NULL)
			{
				// Clear out any existing property
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_CLASS_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_class(_eas2ical_convert_sensitivity_to_class(value));
				icalcomponent_add_property(newEvent, prop);
			}

			// BusyStatus
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_BUSYSTATUS)) != NULL)
			{
				// Clear out any existing property
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_TRANSP_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_transp(_eas2ical_convert_busystatus_to_transp(value));
				icalcomponent_add_property(newEvent, prop);
			}

			// AllDayEvent
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_ALLDAYEVENT)) != NULL)
			{
				// TODO
			}

			// Reminder
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_REMINDER)) != NULL)
			{
				icalcomponent* valarm = NULL;
				struct icaltriggertype trigger;
				
				// Remove any existing VALARM
				if ((valarm = icalcomponent_get_first_component(newEvent, ICAL_VALARM_COMPONENT)) != NULL)
				{
					icalcomponent_remove_component(newEvent, valarm); // We now have ownership of valarm
					icalcomponent_free(valarm);
					valarm = NULL;
				}

				valarm = icalcomponent_new(ICAL_VALARM_COMPONENT);
				
				// TODO: find a way of merging this with the other VALARM creation
				// code to avoid the duplication
			
				// Build an icaltriggertype structure
				trigger = icaltriggertype_from_int(0); // Null the fields first
				trigger.duration.is_neg = 1;
				trigger.duration.minutes = (unsigned int)atoi(value);

				prop = icalproperty_new_action(ICAL_ACTION_DISPLAY);
				icalcomponent_add_property(valarm, prop);

				prop = icalproperty_new_description(ICAL_DEFAULT_REMINDER_NAME); // TODO: make this configurable
				icalcomponent_add_property(valarm, prop);

				prop = icalproperty_new_trigger(trigger);
				icalcomponent_add_property(valarm, prop);

				icalcomponent_add_component(newEvent, valarm);
			}

			// DtStamp
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_DTSTAMP)) != NULL)
			{
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_DTSTAMP_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_dtstamp(icaltime_from_string(value));
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}

			// MeetingStatus
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_MEETINGSTATUS)) != NULL)
			{
				// TODO
			}

			// AppointmentReplyTime
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_APPOINTMENTREPLYTIME)) != NULL)
			{
				// TODO
			}

			// ResponseType
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_RESPONSETYPE)) != NULL)
			{
				// TODO
			}

			// Body
			if ((value = (gchar*)g_hash_table_lookup(exceptionProperties, EAS_ELEMENT_BODY)) != NULL)
			{
				if ((prop = icalcomponent_get_first_property(newEvent, ICAL_DESCRIPTION_PROPERTY)) != NULL)
				{
					icalcomponent_remove_property(newEvent, prop); // Now we have ownership of prop
					icalproperty_free(prop); prop = NULL;
				}
				prop = icalproperty_new_description(value);
				icalcomponent_add_property(newEvent, prop); // vevent takes ownership
			}


			// Finally, destroy the hash and replace the list entry with NULL
			g_hash_table_destroy(exceptionProperties);
			g_slist_nth(exceptionEvents, index)->data = NULL;

			// Add the new event to the parent VCALENDAR
			icalcomponent_add_component(vcalendar, newEvent);

			// Debug output
			g_debug("Added new exception VEVENT to the VCALENDAR:n%s", icalcomponent_as_ical_string(newEvent));
		}
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
	// Variable for the return value
	gchar* result = NULL;

	// Variable for property values as they're read from the XML
	gchar* value  = NULL;

	// Variable to store the TZID value when decoding a <calendar:Timezone> element
	// so we can use it in the rest of the iCal's date/time fields.
	gchar* tzid   = NULL;

	// iCalendar objects
	struct icaltimetype dateTime;
	struct icaltriggertype trigger;

	if (node && (node->type == XML_ELEMENT_NODE) && (!g_strcmp0((char*)(node->name), EAS_ELEMENT_APPLICATIONDATA)))
	{
		xmlNodePtr n = node;

		EasCalInfo* calInfo = NULL;

		// iCalendar objects
		icalcomponent* vcalendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);
		icalcomponent* vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);
		icalcomponent* valarm = icalcomponent_new(ICAL_VALARM_COMPONENT);
		icalcomponent* vtimezone = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);
		icalproperty* prop = NULL;
		icalparameter* param = NULL;
		gboolean isAllDayEvent = FALSE;
		gchar* organizerName = NULL;
		gchar* organizerEmail = NULL;
		GSList* newExceptionEvents = NULL;

		// TODO: get all these strings into constants/#defines
		
		// TODO: make the PRODID configurable somehow
		prop = icalproperty_new_prodid(ICAL_PROPERTY_PRODID);
		icalcomponent_add_property(vcalendar, prop);

		prop = icalproperty_new_version(ICAL_PROPERTY_VERSION);
		icalcomponent_add_property(vcalendar, prop);

		prop = icalproperty_new_method(ICAL_METHOD_PUBLISH);
		icalcomponent_add_property(vcalendar, prop);

		for (n = n->children; n; n = n->next)
		{
			if (n->type == XML_ELEMENT_NODE)
			{
				const gchar* name = (const gchar*)(n->name);

				//
				// Subject
				//
				if (g_strcmp0(name, EAS_ELEMENT_SUBJECT) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_summary(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// StartTime
				//
				else if (g_strcmp0(name, EAS_ELEMENT_STARTTIME) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					dateTime = icaltime_from_string(value);
					if (isAllDayEvent)
					{
						// Ensure time is set to 00:00:00 for all-day events
						dateTime.hour = dateTime.minute = dateTime.second = 0;
					}
					prop = icalproperty_new_dtstart(dateTime);
					if (tzid && strlen(tzid) && !dateTime.is_utc) // Note: TZID not specified if it's a UTC time
					{
						param = icalparameter_new_tzid(tzid);
						icalproperty_add_parameter(prop, param);
					}
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// EndTime
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ENDTIME) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					dateTime = icaltime_from_string(value);
					if (isAllDayEvent)
					{
						// Ensure time is set to 00:00:00 for all-day events
						dateTime.hour = dateTime.minute = dateTime.second = 0;
					}
					prop = icalproperty_new_dtend(dateTime);
					if (tzid && strlen(tzid) && !dateTime.is_utc) // Note: TZID not specified if it's a UTC time
					{
						param = icalparameter_new_tzid(tzid);
						icalproperty_add_parameter(prop, param);
					}
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// DtStamp
				//
				else if (g_strcmp0(name, EAS_ELEMENT_DTSTAMP) == 0)
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
				
				//
				// UID
				//
				else if (g_strcmp0(name, EAS_ELEMENT_UID) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_uid(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// Location
				//
				else if (g_strcmp0(name, EAS_ELEMENT_LOCATION) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_location(value);
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// Body
				//
				else if (g_strcmp0(name, EAS_ELEMENT_BODY) == 0)
				{
					xmlNodePtr subNode = NULL;
					for (subNode = n->children; subNode; subNode = subNode->next)
					{
						if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0((gchar*)subNode->name, EAS_ELEMENT_DATA))
						{
							value = (gchar*)xmlNodeGetContent(subNode);
							prop = icalproperty_new_description(value);
							icalcomponent_add_property(vevent, prop);
							g_free(value); value = NULL;
							break;
						}
					}
				}

				//
				// Sensitivity
				//
				else if (g_strcmp0(name, EAS_ELEMENT_SENSITIVITY) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_class(_eas2ical_convert_sensitivity_to_class(value));
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// BusyStatus
				//
				else if (g_strcmp0(name, EAS_ELEMENT_BUSYSTATUS) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new_transp(_eas2ical_convert_busystatus_to_transp(value));
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
				}

				//
				// Categories
				//
				else if (g_strcmp0(name, EAS_ELEMENT_CATEGORIES) == 0)
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

				//
				// Reminder
				//
				else if (g_strcmp0(name, EAS_ELEMENT_REMINDER) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);

					// Build an icaltriggertype structure
					trigger = icaltriggertype_from_int(0); // Null the fields first
					trigger.duration.is_neg = 1;
					trigger.duration.minutes = (unsigned int)atoi(value);

					prop = icalproperty_new_action(ICAL_ACTION_DISPLAY);
					icalcomponent_add_property(valarm, prop);

					prop = icalproperty_new_description(ICAL_DEFAULT_REMINDER_NAME); // TODO: make this configurable
					icalcomponent_add_property(valarm, prop);

					prop = icalproperty_new_trigger(trigger);
					icalcomponent_add_property(valarm, prop);

					g_free(value); value = NULL;
				}

				//
				// AllDayEvent
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ALLDAYEVENT) == 0)
				{
					value = (gchar*)xmlNodeGetContent(n);
					if (atoi(value) == 1)
					{
						isAllDayEvent = TRUE;

						// Check if we've already stored any start/end dates, and if so ensure all their times are set to 00:00:00.
						// (Note: DtStamp times shouldn't be affected by this as that's the time the calendar entry was created,
						// not when the event starts or ends.)
						for (prop = icalcomponent_get_first_property(vevent, ICAL_DTEND_PROPERTY); prop;
							 prop = icalcomponent_get_next_property(vevent, ICAL_DTEND_PROPERTY))
						{
							struct icaltimetype date = icalproperty_get_dtend(prop);
							date.hour = date.minute = date.second = 0;
							icalproperty_set_dtend(prop, dateTime);
						}
						for (prop = icalcomponent_get_first_property(vevent, ICAL_DTSTART_PROPERTY); prop;
							 prop = icalcomponent_get_next_property(vevent, ICAL_DTSTART_PROPERTY))
						{
							struct icaltimetype date = icalproperty_get_dtstart(prop);
							date.hour = date.minute = date.second = 0;
							icalproperty_set_dtstart(prop, dateTime);
						}
					}
					g_free(value); value = NULL;
				}

				//
				// OrganizerName
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ORGANIZER_NAME) == 0)
				{
					organizerName = (gchar*)xmlNodeGetContent(n);
					// That's all for now: deal with it after the loop completes so we
					// have both OrganizerName and OrganizerEmail
				}

				//
				// OrganizerEmail
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ORGANIZER_EMAIL) == 0)
				{
					organizerEmail = (gchar*)xmlNodeGetContent(n);
					// That's all for now: deal with it after the loop completes so we
					// have both OrganizerName and OrganizerEmail
				}

				//
				// Attendees
				//
				else if (g_strcmp0(name, EAS_ELEMENT_ATTENDEES) == 0)
				{
					_eas2ical_process_attendees(n, vevent);
				}

				//
				// TimeZone
				//
				else if (g_strcmp0(name, EAS_ELEMENT_TIMEZONE) == 0)
				{
					_eas2ical_process_timezone(n, vtimezone, &tzid);
				}

				//
				// Recurrence
				//
				else if (g_strcmp0(name, EAS_ELEMENT_RECURRENCE) == 0)
				{
					_eas2ical_process_recurrence(n, vevent);
				}

				//
				// Exceptions
				//
				else if (g_strcmp0(name, EAS_ELEMENT_EXCEPTIONS) == 0)
				{
					newExceptionEvents = _eas2ical_process_exceptions(n, vevent);
					// This is dealt with below...
				}

				//
				// Unmapped data fields
				//
				else
				{
					// Build a new custom property called X-MEEGO-ACTIVESYNCD-{ElementName}
					gchar* propertyName = g_strconcat(ICAL_EXTENSION_PROPERTY_PREFIX, name, NULL);
					value = (gchar*)xmlNodeGetContent(n);
					prop = icalproperty_new(ICAL_X_PROPERTY);

					g_debug("Found EAS element that doesn't map to a VEVENT property. Creating X property %s:%s", propertyName, value);

					icalproperty_set_x_name(prop, propertyName);
					icalproperty_set_value(prop, icalvalue_new_from_string(ICAL_X_VALUE, value));
					icalcomponent_add_property(vevent, prop);
					g_free(value); value = NULL;
					g_free(propertyName); propertyName = NULL;
				}
			}
		}

		// Deal with OrganizerName and OrganizerEmail
		if (organizerEmail)
		{
		   prop = icalproperty_new_organizer(organizerEmail);
		   g_free(organizerEmail); organizerEmail = NULL;

		   if (organizerName)
		   {
			   param = icalparameter_new_cn(organizerName);
			   icalproperty_add_parameter(prop, param);
			   g_free(organizerName); organizerName = NULL;
		   }

		   icalcomponent_add_property(vevent, prop);
		}

		// Check organizerName again, so we free it if we had a name but no e-mail
		if (organizerName)
		{
		   // TODO: Is there any way we can use the name without the e-mail address?
		   g_warning("OrganizerName element found but no OrganizerEmail");
		   g_free(organizerName); organizerName = NULL;
		}


		// Add the subcomponents to their parent components
        if (icalcomponent_count_properties(vtimezone, ICAL_ANY_PROPERTY) > 0)
        {
			icalcomponent_add_component(vcalendar, vtimezone);
		}
		icalcomponent_add_component(vcalendar, vevent);
        if (icalcomponent_count_properties(valarm, ICAL_ANY_PROPERTY) > 0)
        {
			icalcomponent_add_component(vevent, valarm);
		}

		// Now handle any non-trivial exception events we found in the <Exceptions> element
		if (newExceptionEvents)
		{
			_eas2ical_add_exception_events(vcalendar, vevent, newExceptionEvents);
			// _eas2ical_add_exception_events() destroys the hash tables as it goes
			// so all we need to do here is free the list
			g_slist_free(newExceptionEvents);
		}

		// Now insert the server ID and iCalendar into an EasCalInfo object and serialise it
		calInfo = eas_cal_info_new();
		calInfo->icalendar = (gchar*)icalcomponent_as_ical_string_r(vcalendar); // Ownership passes to the EasCalInfo
		calInfo->server_id = (gchar*)server_id;
		if (!eas_cal_info_serialise(calInfo, &result))
		{
			// TODO: log error
			result = NULL;
		}

		// Free the EasCalInfo GObject
		g_object_unref(calInfo);

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


/**
 * \brief Process the RRULE (recurrence rule) property during parsing of an iCalendar VEVENT component
 *
 * \param prop     Pointer to the RRULE property
 * \param appData  Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_rrule(icalproperty* prop, xmlNodePtr appData)
{
	// Get the iCal RRULE property
	struct icalrecurrencetype rrule = icalproperty_get_rrule(prop);

	// Create a new <Recurrence> element to contain the recurrence sub-elements
	xmlNodePtr recurNode = xmlNewChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_RECURRENCE, NULL);

	// Other declarations
	int    recurType    = 0;
	gchar* xmlValue     = NULL;
	int    index        = 0;
	guint  dayOfWeek    = 0;
	gint   weekOfMonth  = 0;
	gint   monthOfYear  = 0;
	gint   dayOfMonth   = 0;

	//
	// Type element
	//
	switch (rrule.freq)
	{
		case ICAL_SECONDLY_RECURRENCE:
		case ICAL_MINUTELY_RECURRENCE:
		case ICAL_HOURLY_RECURRENCE:
			g_warning("DATA LOSS: cannot encode secondly/minutely/hourly recurrence in EAS.");
			break;
		case ICAL_DAILY_RECURRENCE:
			recurType = 0;
			break;
		case ICAL_WEEKLY_RECURRENCE:
			recurType = 1;
			break;
		case ICAL_MONTHLY_RECURRENCE:
			recurType = 2;
			break;
		case ICAL_YEARLY_RECURRENCE:
			recurType = 5;
			break;
		case ICAL_NO_RECURRENCE:
		default:
			g_warning("RRULE with no recurrence type.");
			break;
	}
	// Note: don't add this to the XML yet: if we encounter an "nth day in the month" value
	// blow we need to change this

	//
	// COUNT & UNTIL
	//
	// Note: count and until are mutually exclusive in both formats, with count taking precedence
	if (rrule.count)
	{
		// EAS specifies a maximum value of 999 for the Occurrences element
		if (rrule.count > 999)
		{
			g_warning("DATA LOSS: RRULE had recurrence count of %d, maximum is 999.", rrule.count);
			rrule.count = 999;
		}
		xmlValue = g_strdup_printf("%d", rrule.count);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_OCCURRENCES, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}
	else if (!icaltime_is_null_time(rrule.until))
	{
		// Note: icaltime_as_ical_string() retains ownership of the string, so no need to free
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_UNTIL, (const xmlChar*)icaltime_as_ical_string(rrule.until));
	}

	//
	// INTERVAL
	//
	if (rrule.interval)
	{
		// EAS specifies a maximum value of 999 for the Interval element
		if (rrule.interval > 999)
		{
			g_warning("DATA LOSS: RRULE had recurrence interval of %d, maximum is 999.", rrule.interval);
			rrule.interval = 999;
		}
		// Only write the Interval element if it's greater than 1;
		// 1 is te default (i.e. every day)
		if (rrule.interval > 1)
		{
			xmlValue = g_strdup_printf("%d", rrule.interval);
			xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_INTERVAL, (const xmlChar*)xmlValue);
			g_free(xmlValue); xmlValue = NULL;
		}
	}

	// 
	// BYDAY
	//
	// icalrecurrencetype arrays are terminated with ICAL_RECURRENCE_ARRAY_MAX unless they're full
	for (index = 0;
	     (index < ICAL_BY_DAY_SIZE) && (rrule.by_day[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++)
	{
		enum icalrecurrencetype_weekday icalDayOfWeek = icalrecurrencetype_day_day_of_week(rrule.by_day[index]);
		gint icalDayPosition = icalrecurrencetype_day_position(rrule.by_day[index]);

		switch (icalDayOfWeek)
		{
			case ICAL_SUNDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_SUNDAY;
				break;
			case ICAL_MONDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_MONDAY;
				break;
			case ICAL_TUESDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_TUESDAY;
				break;
			case ICAL_WEDNESDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_WEDNESDAY;
				break;
			case ICAL_THURSDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_THURSDAY;
				break;
			case ICAL_FRIDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_FRIDAY;
				break;
			case ICAL_SATURDAY_WEEKDAY:
				dayOfWeek |= DAY_OF_WEEK_SATURDAY;
				break;
			case ICAL_NO_WEEKDAY:
			default:
				g_warning("Found by-day RRULE with an empty day value");
				break;
		}

		// Now process the position part
		if (icalDayPosition != 0)
		{
			if (icalDayPosition < -1)
			{
				g_warning("DATA LOSS: EAS cannot encode RRULE position value of %d", icalDayPosition);
				// For now, convert all large naegative values (meaning nth from the end of the month)
				// to 5 (meaning last of the month)
				icalDayPosition = 5;
			}
			else if (icalDayPosition == -1)
			{
				// Convert to the equivalent EAS value
				// (both mean "last instance in the month")
				icalDayPosition = 5;
			}

			// Check if we've already processed a position part from one of the other recurrence days
			// (EAS has no way of encoding different position values for different days)
			if (weekOfMonth && (weekOfMonth != icalDayPosition))
			{
				g_warning("DATA LOSS: Position %d already stored for this recurrence; ignoring value of %d", weekOfMonth, icalDayPosition);
			}
			else
			{
				weekOfMonth = icalDayPosition;
			}
	    }
	}// end of for loop

	if (dayOfWeek)
	{
		//g_debug("RECURRENCE: DayOfWeek value = %d (0x%08X)", dayOfWeek, dayOfWeek);
		xmlValue = g_strdup_printf("%d", dayOfWeek);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DAYOFWEEK, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}
	if (weekOfMonth)
	{
		// Set the Type value to 3 ("Recurs monthly on the nth day")
		recurType = 3;
		
		xmlValue = g_strdup_printf("%d", weekOfMonth);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_WEEKOFMONTH, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}

	// And now we can add the Type element too
	xmlValue = g_strdup_printf("%d", recurType);
	xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_TYPE, (const xmlChar*)xmlValue);
	g_free(xmlValue); xmlValue = NULL;

	//
	// WKST
	//
	if (rrule.week_start)
	{
		// EAS value is 0=Sunday..6=Saturday
		// libical value is 0=NoDay, 1=Sunday..7=Saturday
		xmlValue = g_strdup_printf("%d", rrule.week_start - 1);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_FIRSTDAYOFWEEK, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}

	//
	// BYMONTH
	//
	for (index = 0;
	     (index < ICAL_BY_MONTH_SIZE) && (rrule.by_month[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++)
	{
		if (monthOfYear == 0)
		{
			monthOfYear = rrule.by_month[index];
		}
		else
		{
			// We've already set monthOfyear: EAS only supports a single monthly recurrence
			// (unlike days where we can repeat on many days of the week)
			g_warning("DATA LOSS: Already set to recur on month %d, discarding recurrence info for month %d", monthOfYear, rrule.by_month[index]);
		}
	}
	if (monthOfYear > 0)
	{
		xmlValue = g_strdup_printf("%d", monthOfYear);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_MONTHOFYEAR, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}
	
	//
	// BYMONTHDAY
	//
	for (index = 0;
	     (index < ICAL_BY_MONTHDAY_SIZE) && (rrule.by_month_day[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++)
	{
		if (dayOfMonth == 0)
		{
			dayOfMonth = rrule.by_month_day[index];
		}
		else
		{
			// EAS only supports a single occurrence of DayOfMonth
			g_warning("DATA LOSS: Already set to recur on day %d of the month, discarding recurrence info for day %d of month", dayOfMonth, rrule.by_month_day[index]);
		}
	}
	if (dayOfMonth > 0)
	{
		xmlValue = g_strdup_printf("%d", dayOfMonth);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DAYOFMONTH, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	}
}


/**
 * \brief Process the VEVENT component during parsing of an iCalendar
 *
 * \param vevent  Pointer to the iCalendar VEVENT component
 * \param appData Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_vevent(icalcomponent* vevent, xmlNodePtr appData)
{
	if (vevent)
	{
		xmlNodePtr categories = NULL;
		xmlNodePtr exceptions = NULL;
		xmlNodePtr icalExtns = NULL;
		struct icaltimetype startTime, endTime;
		
		icalproperty* prop;
		for (prop = icalcomponent_get_first_property(vevent, ICAL_ANY_PROPERTY);
			 prop;
			 prop = icalcomponent_get_next_property(vevent, ICAL_ANY_PROPERTY))
		{
			const icalproperty_kind prop_type = icalproperty_isa(prop);
			switch (prop_type)
			{
				// SUMMARY
				case ICAL_SUMMARY_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SUBJECT, (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
					
				// DTSTAMP
				case ICAL_DTSTAMP_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DTSTAMP, (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
					
				// DTSTART
				case ICAL_DTSTART_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_STARTTIME, (const xmlChar*)icalproperty_get_value_as_string(prop));
					// And additionally store the start time so we can calculate the AllDayEvent value later
					startTime = icalproperty_get_dtstart(prop);
					break;
					
				// DTEND
				case ICAL_DTEND_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ENDTIME, (const xmlChar*)icalproperty_get_value_as_string(prop));
					// And additionally store the end time so we can calculate the AllDayEvent value later
					endTime = icalproperty_get_dtend(prop);
					break;
					
				// LOCATION
				case ICAL_LOCATION_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_LOCATION, (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
					
				// UID
				case ICAL_UID_PROPERTY:
					xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_UID, (const xmlChar*)icalproperty_get_value_as_string(prop));
					break;
					
				// CLASS
				case ICAL_CLASS_PROPERTY:
					{
						icalproperty_class classValue = icalproperty_get_class(prop);
						switch (classValue)
						{
						case ICAL_CLASS_CONFIDENTIAL:
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*)EAS_SENSITIVITY_CONFIDENTIAL);
							break;
						case ICAL_CLASS_PRIVATE:
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*)EAS_SENSITIVITY_PRIVATE);
							break;
						default: // PUBLIC or NONE (iCalendar doesn't distinguish between 0 (Normal) and 1 (Personal))
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*)EAS_SENSITIVITY_NORMAL);
							break;
						}
					}
					break;
					
				// TRANSP
				case ICAL_TRANSP_PROPERTY:
					{
						icalproperty_transp transp = icalproperty_get_transp(prop);
						if (transp == ICAL_TRANSP_TRANSPARENT)
						{
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_BUSYSTATUS, (const xmlChar*)EAS_BUSYSTATUS_FREE);
						}
						else // OPAQUE
						{
							// iCalendar doesn't distinguish between 1 (Tentative), 2 (Busy), 3 (Out of Office)
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_BUSYSTATUS, (const xmlChar*)EAS_BUSYSTATUS_BUSY);
						}
					}
					break;
					
				// CATEGORIES
				case ICAL_CATEGORIES_PROPERTY:
					{
						if (categories == NULL)
						{
							categories = xmlNewChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_CATEGORIES, NULL);
						}
						xmlNewTextChild(categories, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_CATEGORY, (const xmlChar*)icalproperty_get_value_as_string(prop));
					}
					break;
					
				// ORGANIZER
				case ICAL_ORGANIZER_PROPERTY:
					{
						icalparameter* cnParam = NULL;
						
						// Get the e-mail address
						xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_EMAIL, (const xmlChar*)icalproperty_get_organizer(prop));

						// Now check for a name in the (optional) CN parameter
						cnParam = icalproperty_get_first_parameter(prop, ICAL_CN_PARAMETER);
						if (cnParam)
						{
							xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_NAME, (const xmlChar*)icalparameter_get_cn(cnParam));
						}
					}
					break;
					
				// RRULE
				case ICAL_RRULE_PROPERTY:
					{
						_ical2eas_process_rrule(prop, appData);
					}
					break;

				// RDATE
				case ICAL_RDATE_PROPERTY:
					{
						// TODO...
					}
					break;

				// EXDATE
				case ICAL_EXDATE_PROPERTY:
					{
						// EXDATE consists of a list of date/times, comma separated.
						// However, libical breaks this up for us and converts it into
						// a number of single-value properties.
						
						xmlNodePtr exception = NULL;

						// Create the <Exceptions> container element if not already present
						if (exceptions == NULL)
						{
							exceptions = xmlNewChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS, NULL);
						}

						// Now create the <Exception> element
						exception = xmlNewChild(exceptions, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTION, NULL);
						xmlNewTextChild(exception, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DELETED, (const xmlChar*)EAS_BOOLEAN_TRUE);
						xmlNewTextChild(exception, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONSTARTTIME, (const xmlChar*)icalproperty_get_value_as_string(prop));
					}
					break;

				// DESCRIPTION
				case ICAL_DESCRIPTION_PROPERTY:
					{
						// TODO...
					}
					break;

				default:
					{
						// Any other unmapped properties get stored as extension elements under the
						// <activesyncd:iCalExtensions> collection element.
						
						gchar* elementName = NULL;

						// Create the collecion element when first needed
						if (icalExtns == NULL)
						{
							icalExtns = xmlNewChild(appData, NULL, (const xmlChar*)X_NAMESPACE_ACTIVESYNCD X_ELEMENT_ICALEXENSIONS, NULL);
						}
					
						// Note: icalproperty_as_ical_string() keeps ownership of the string so we don't have to delete
						g_debug("Found unsupported iCalendar property (%s): adding as extension element under <iCalExtensions>", icalproperty_as_ical_string(prop));

						elementName = g_strconcat(X_NAMESPACE_ACTIVESYNCD, icalproperty_get_property_name(prop), NULL);
						xmlNewTextChild(icalExtns, NULL, (const xmlChar*)elementName, (const xmlChar*)icalproperty_get_value_as_string(prop));
						g_free(elementName); elementName = NULL;
					}
					break;
			}// end of switch
		}// end of for loop

		// Add an <AllDayEvent> element if both the start and end dates have no times
		// (ie. just dates, with time set to midnight) and are 1 day apart
		// (from [MS-ASCAL]: "An item marked as an all day event is understood to begin
		// on midnight of the current day and to end on midnight of the next day.")
		if (icaltime_is_null_time(startTime) &&
		    icaltime_is_null_time(endTime) &&
		    (icaltime_as_timet(endTime) - icaltime_as_timet(startTime)) == (time_t)SECONDS_PER_DAY)
		{
			xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ALLDAYEVENT, (const xmlChar*)EAS_BOOLEAN_TRUE);
		}
		else
		{
			xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ALLDAYEVENT, (const xmlChar*)EAS_BOOLEAN_FALSE);
		}
	}
}


/**
 * \brief Process the VALARM component during parsing of an iCalendar
 *
 * \param valarm  Pointer to the iCalendar VALARM component
 * \param appData Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_valarm(icalcomponent* valarm, xmlNodePtr appData)
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

			xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_REMINDER, (const xmlChar*)minutes_buf);
		}
	}
}


/**
 * \brief Parse the STANDARD and DAYLIGHT subcomponents of VTIMEZONE.
 *        Using one function for both as their formats are identical.
 *
 * \param subcomponent The STANDARD or DAYLIGHT subcomponent of the VTIMEZONE component
 * \param timezone     The EAS timezone structure to parse this subcomponent into
 * \param type         Determineds whether subcomponent is a STANDARD or a DAYLIGHT
 */
static void _ical2eas_process_xstandard_xdaylight(icalcomponent* subcomponent, EasTimeZone* timezone, icalcomponent_kind type)
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
			// We can assume FREQ=YEARLY: EAS only supports annually recurring timezone changes
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
 * \brief Process the VTIMEZONE component during parsing of an iCalendar
 *
 * \param vtimezone  Pointer to the iCalendar VTIMEZONE component
 * \param appData    Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_vtimezone(icalcomponent* vtimezone, xmlNodePtr appData)
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
			_ical2eas_process_xstandard_xdaylight(subcomponent, &timezoneStruct, icalcomponent_isa(subcomponent));
		}

		// Write the timezone into the XML, base64-encoded
		timezoneBase64 = g_base64_encode((const guchar *)(&timezoneStruct), sizeof(EasTimeZone));
		xmlNewTextChild(appData, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_TIMEZONE, (const xmlChar*)timezoneBase64);
		g_free(timezoneBase64);
	}
}


/**
 * \brief Parse an iCalendar document and convert to EAS XML format
 *
 * \param doc      REDUNDANT PARAMETER: only required for debug output. TODO: remove this
 * \param appData  The top-level <ApplicationData> XML element in which to store all the parsed elements
 * \param calInfo  The EasCalInfo struct containing the iCalendar string to parse (plus a server ID)
 */
gboolean eas_cal_info_translator_parse_request(xmlDocPtr doc, xmlNodePtr appData, EasCalInfo* calInfo)
{
	gboolean success = FALSE;
	icalcomponent* ical = NULL;
	
	if (doc &&
	    appData &&
	    calInfo &&
	    (appData->type == XML_ELEMENT_NODE) &&
	    (g_strcmp0((char*)(appData->name), EAS_ELEMENT_APPLICATIONDATA) == 0) &&
	    (ical = icalparser_parse_string(calInfo->icalendar)) &&
	    (icalcomponent_isa(ical) == ICAL_VCALENDAR_COMPONENT))
	{
		icalcomponent* vevent = NULL;
		
		// Process the components of the VCALENDAR
		_ical2eas_process_vtimezone(icalcomponent_get_first_component(ical, ICAL_VTIMEZONE_COMPONENT), appData);
		vevent = icalcomponent_get_first_component(ical, ICAL_VEVENT_COMPONENT);
		_ical2eas_process_vevent(vevent, appData);
		_ical2eas_process_valarm(icalcomponent_get_first_component(vevent, ICAL_VALARM_COMPONENT), appData);

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

