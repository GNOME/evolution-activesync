/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "eas-cal-info-translator.h"

#include <libical/ical.h>

#include <wbxml/wbxml.h>

// Values for converting icaldurationtype into a number of minutes.
// Note that code using these values is almost certainly wrong.
// Days involving daylight saving time changes will have 23 or 25
// hours, etc.
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

/* summary for synthesized events, see eas_cal_info_translator_parse_request() */
#define ACTIVESYNCD_PSEUDO_EVENT "[[activesyncd pseudo event - ignore me]]"

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
#define EAS_ELEMENT_DTSTAMP                   "DTStamp"
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
#define EAS_ELEMENT_TYPE                      "Recurrence_Type"
#define EAS_ELEMENT_OCCURRENCES               "Recurrence_Occurrences"
#define EAS_ELEMENT_INTERVAL                  "Recurrence_Interval"
#define EAS_ELEMENT_WEEKOFMONTH               "Recurrence_WeekOfMonth"
#define EAS_ELEMENT_DAYOFWEEK                 "Recurrence_DayOfWeek"
#define EAS_ELEMENT_MONTHOFYEAR               "Recurrence_MonthOfYear"
#define EAS_ELEMENT_UNTIL                     "Recurrence_Until"
#define EAS_ELEMENT_DAYOFMONTH                "Recurrence_DayOfMonth"
#define EAS_ELEMENT_CALENDARTYPE              "CalendarType"
#define EAS_ELEMENT_ISLEAPMONTH               "IsLeapMonth"
#define EAS_ELEMENT_FIRSTDAYOFWEEK            "FirstDayOfWeek"
#define EAS_ELEMENT_EXCEPTIONS                "Exceptions"
#define EAS_ELEMENT_EXCEPTION                 "Exception"
#define EAS_ELEMENT_DELETED                   "Exception_Deleted"
#define EAS_ELEMENT_EXCEPTIONSTARTTIME        "Exception_StartTime"
#define EAS_ELEMENT_APPOINTMENTREPLYTIME      "AppointmentReplyTime"
#define EAS_ELEMENT_RESPONSETYPE              "ResponseType"
#define EAS_ELEMENT_BODY                      "Body"
#define EAS_ELEMENT_RESPONSEREQUESTED         "ResponseRequested"
#define EAS_ELEMENT_NATIVEBODYTYPE            "NativeBodyType"
#define EAS_ELEMENT_ONLINEMEETINGCONFLINK     "OnlineMeetingConfLink"
#define EAS_ELEMENT_ONLINEMEETINGEXTERNALLINK "OnlineMeetingExternalLink"
#define EAS_ELEMENT_DATA                      "Data"
#define EAS_ELEMENT_TRUNCATED                 "Truncated"
#define EAS_ELEMENT_BODY_TYPE                 "Type"

#define EAS_SENSITIVITY_NORMAL                "0"
#define EAS_SENSITIVITY_PERSONAL              "1"
#define EAS_SENSITIVITY_PRIVATE               "2"
#define EAS_SENSITIVITY_CONFIDENTIAL          "3"

#define EAS_BUSYSTATUS_FREE                   "0"
#define EAS_BUSYSTATUS_TENTATIVE              "1"
#define EAS_BUSYSTATUS_BUSY                   "2"
#define EAS_BUSYSTATUS_OUTOFOFFICE            "3"

#define EAS_BODY_TYPE_PLAINTEXT               "1"

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
} __attribute__ ( (packed)) EasSystemTime;

compile_time_assert ( (sizeof (EasSystemTime) == 16), EasSystemTime_not_expected_size);

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
} __attribute__ ( (packed)) EasTimeZone;	// 172

compile_time_assert ( (sizeof (EasTimeZone) == 172), EasTimeZone_not_expected_size);

/* Check if a contact field allready set in the applicationdata xml children*/
static gboolean
is_element_set (xmlNodePtr appData, const gchar* name)
{
	xmlNodePtr node = NULL;
	g_return_val_if_fail (appData != NULL && name != NULL, FALSE);
	node = appData;
	for (node = appData->children; node ; node = node->next)
		if (!strcmp ( (char*) node->name, name))
			return TRUE;

	return FALSE;
}

/**     @brief Get a DATE or DATE-TIME property as an icaltime
 *
 *      If the property is a DATE-TIME with a timezone parameter and a
 *      corresponding VTIMEZONE is present in the component, the
 *      returned component will already be in the correct timezone;
 *      otherwise the caller is responsible for converting it.
 *
 *      FIXME this is useless until we can flag the failure
 *
 *      A copy of icalcomponent_get_datetime(), which is not part of the libical API.
 */
static struct icaltimetype
_eas2cal_get_datetime (const icalcomponent *comp, const icalproperty *prop) {

    const icalcomponent *c;
    icalparameter      *param;
    struct icaltimetype ret;

    ret = icalvalue_get_datetime(icalproperty_get_value(prop));

    if ((param = icalproperty_get_first_parameter((icalproperty *)prop, ICAL_TZID_PARAMETER))
        != NULL) {
        const char     *tzid = icalparameter_get_tzid(param);
        icaltimezone   *tz = NULL;

        for (c = comp; c != NULL; c = icalcomponent_get_parent((icalcomponent *)c)) {
            tz = icalcomponent_get_timezone((icalcomponent *)c, tzid);
            if (tz != NULL)
                break;
        }

        if (tz == NULL)
            tz = icaltimezone_get_builtin_timezone_from_tzid(tzid);

        if (tz != NULL)
            ret = icaltime_set_timezone(&ret, tz);
    }

    return ret;
}

/** missing in libical header files?! */
extern icalcomponent *icalproperty_get_parent (const icalproperty *prop);

/**
 * In contrast to icalproperty_get_recurrenceid(), this function sets the
 * time zone of the returned value based on the TZID and VTIMEZONE of
 * the calender in which the property is set.
 */
static struct icaltimetype
_eas2cal_property_get_recurrenceid (const icalproperty *prop)
{
	return _eas2cal_get_datetime (icalproperty_get_parent (prop),
				      prop);
}

static struct icaltimetype
_eas2cal_component_get_recurrenceid (const icalcomponent *comp)
{
	icalproperty *prop = icalcomponent_get_first_property ((icalcomponent *)comp, ICAL_RECURRENCEID_PROPERTY);
	return prop ?
		_eas2cal_get_datetime (comp, prop) :
		icaltime_null_time();
}

/**
 * Convert a <Sensitivity> value from EAS XML into an iCal CLASS property value
 */
static icalproperty_class _eas2ical_convert_sensitivity_to_class (const gchar* sensitivityValue)
{
	if (g_strcmp0 (sensitivityValue, EAS_SENSITIVITY_CONFIDENTIAL) == 0) {
		return ICAL_CLASS_CONFIDENTIAL;
	} else if (g_strcmp0 (sensitivityValue, EAS_SENSITIVITY_PRIVATE) == 0) {
		return ICAL_CLASS_PRIVATE;
	} else { // Personal or Normal (iCal doesn't distinguish between them)
		return ICAL_CLASS_PUBLIC;
	}
}


/**
 * Convert a <BusyStatus> value from EAS XML into an iCal TRANSP property value
 */
static icalproperty_transp _eas2ical_convert_busystatus_to_transp (const gchar* busystatusValue)
{
	if (g_strcmp0 (busystatusValue, EAS_BUSYSTATUS_FREE) == 0) {
		return ICAL_TRANSP_TRANSPARENT;
	} else { // Tentative, Busy or Out of Office
		return ICAL_TRANSP_OPAQUE;
	}
}


/**
 * The by_day member of icalrecurrencetype is a composite value, encoding
 * both the day to repeat on and (in the case of monthly recurrence) the position
 * of the day to repeat on (e.g. 2nd Monday in each month). libical doesn't provide
 * an API to encode this, so we've had to write our own by reverse engineering
 * the implementation of icalrecurrencetype_day_day_of_week() and
 * icalrecurrencetype_day_position() in icalrecur.c.
 *
 * @param byDayDayOfWeek
 *      The day of the week to repeat on
 * @param byDayPos
 *      The position of this day in the month on which to repeat (or 0 if not applicable)
 */
static short _eas2ical_make_rrule_by_day_value (icalrecurrencetype_weekday byDayDayOfWeek, short byDayPos)
{
	// Now calculate the composite value as follows:
	//  - Multiply byDayPos by 8
	//  - Add byDayDayOfWeek if byDayPos is positive, or subtract if negative
	short dowShort = (short) byDayDayOfWeek;
	return
		(byDayPos * 8) +
		( (byDayPos >= 0) ? dowShort : (-1 * dowShort));
}


/**
 * Private function to decode an EasSystemTime struct containing details of a recurring
 * timezone change pattern, modify it to a format iCalendar expects and create the
 * required RRULE property value.
 *
 * @param  date
 *      Pointer to the EasSystemTime struct. This will be modified in this
 *      function to contain the first instance of the recurring sequence.
 * @param  rrule
 *      Pointer to an icalrecurrencetype struct to store the recurrence rule in.
 */
static void _eas2ical_convert_relative_timezone_date (EasSystemTime* date, struct icalrecurrencetype* rrule)
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
	// the modifications on then assign it back before we return.
	memcpy (&modifiedDate, date, sizeof (EasSystemTime));

	modifiedDate.Year = EPOCH_START_YEAR;

	// Now adjust the DayOfWeek.
	// SYSTEMTIME's wDayOfWeek has 0 = Sunday, 1 = Monday,...     http://msdn.microsoft.com/en-us/library/ms724950(v=vs.85).aspx
	// GDateWeekDay has 0 = G_DATE_BAD_WEEKDAY, 1 = Monday,...    http://developer.gnome.org/glib/stable/glib-Date-and-Time-Functions.html#GDateWeekday
	if (modifiedDate.DayOfWeek == 0) {
		modifiedDate.DayOfWeek = (guint16) G_DATE_SUNDAY;
		// Now our DayOfWeek matches GDateWeekDay (see below)
	}

	// date->DayOfWeek will give us the day to repeat on
	// date->Day will give us the nth occurrence in the month of this day
	// (where 5 == last, even if there are only 4)

	// We're going to build a GDate object to calculate the actual date in 1970 that represents
	// the first instance of this recurrence pattern.
	// Start by initialising to the 1st of of the month
	recurrenceStartDate = g_date_new_dmy (1, modifiedDate.Month, modifiedDate.Year);

	// Now seek for the first occurrence of the day the sequence repeats on.
	weekday = (GDateWeekday) modifiedDate.DayOfWeek;
	while (g_date_get_weekday (recurrenceStartDate) != weekday) {
		g_date_add_days (recurrenceStartDate, 1);
	}

	// Now we've got the FIRST occurence of the correct weekday in the correct month in 1970.
	// Finally we need to seek for the nth occurrence (where 5th = last, even if there are only 4)
	occurrence = 1;
	while (occurrence++ < date->Day) {
		g_date_add_days (recurrenceStartDate, DAYS_PER_WEEK);

		// Check we havn't overrun the end of the month
		// (If we have, just roll back and we're done: we're on the last occurrence)
		if (g_date_get_month (recurrenceStartDate) != modifiedDate.Month) {
			g_date_subtract_days (recurrenceStartDate, DAYS_PER_WEEK);
			break;
		}
	}

	modifiedDate.Day = g_date_get_day (recurrenceStartDate);

	g_free (recurrenceStartDate);

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
	byDayPos = (short) date->Day;
	if (byDayPos == 5) {
		byDayPos = -1;
	}
	// Here the day is represented by the enum icalrecurrencetype_weekday, which has the
	// range 0 = ICAL_NO_WEEKDAY, 1 = ICAL_SUNDAY_WEEKDAY, 2 = ICAL_MONDAY_WEEKDAY...
	// so we can just add 1 to the EasSystemTime value (see above).
	byDayDayOfWeek = (icalrecurrencetype_weekday) (date->DayOfWeek + 1);

	// Now calculate the composite value as follows:
	//  - Multiply byDayPos by 8
	//  - Add byDayDayOfWeek if byDayPos is positive, or subtract if negative
	rrule->by_day[0] = _eas2ical_make_rrule_by_day_value (byDayDayOfWeek, byDayPos);

	// Finally, copy the modified version back into the date that was passed in
	memcpy (date, &modifiedDate, sizeof (EasSystemTime));
}

/**
 * Turn UTC date-time value into time relative to zone (if given),
 * strip down to date-only (if all-day). Add TZID as needed.
 *
 * @return TRUE if zone definition is needed
 */
static gboolean _eas2ical_convert_datetime_property(icalproperty *prop,
						    icaltimezone *icaltz,
						    gboolean isAllDayEvent,
						    struct icaltimetype (*get)(const icalproperty *prop),
						    void (*set)(icalproperty *prop, struct icaltimetype))
{
	gboolean needtz = FALSE;
	// NOP without either of these
	if (icaltz || isAllDayEvent) {
		struct icaltimetype tt = get (prop);
		if (icaltz) {
			struct icaltimetype localtt = icaltime_convert_to_zone (tt, icaltz);
			// Sanity check for all day events: should align with midnight after conversion.
			// If it doesn't, something is wrong. Happened with Exchange 2010
			// on 123together.com, but only for events created via ActiveSync,
			// not for events created via OWA. If the check fails, then don't
			// do the conversion to local time.
			if (isAllDayEvent) {
				if (localtt.hour || localtt.minute || localtt.second)
					g_warning("All day event does not start/finish at local midnight: %s.  Reverting to EAS specified time: %s",
						  icaltime_as_ical_string(localtt),
						  icaltime_as_ical_string(tt));
				else tt=localtt;
			} else tt=localtt;
		} else {
			if (tt.hour || tt.minute || tt.second)
				g_warning("All day event with no timezone does not start at UTC midnight: %s", icaltime_as_ical_string(tt));
		}
		// If the sanity check (see above) failed, don't mark as a date
		if (isAllDayEvent && !tt.hour && !tt.minute && !tt.second)
			tt.is_date = 1;
		set (prop, tt);

		// Date-based events can and should be defined in local time without time zone.
		// UTC time stamps don't need a TZID.
		if (icaltz &&
		    !tt.is_date &&
		    !icaltime_is_utc (tt)) {
			const char *tzid = icaltimezone_get_tzid (icaltz);
			if (tzid && strlen (tzid)) {
				icalparameter *param = icalparameter_new_tzid (tzid);
				icalproperty_add_parameter (prop, param);
				needtz = TRUE;
			}
		}
	}
	return needtz;
}

/**
 * Fix all date-time values (DTSTART/END, RECURRENCE-ID, EXDATE)
 * and the RRULE UNTIL clause.
 *
 * @return TRUE if zone definition is needed
 */
static gboolean _eas2ical_convert_component(icalcomponent *vevent,
					    icaltimezone *icaltz,
					    gboolean isAllDayEvent,
					    gboolean parentIsAllDayEvent)
{
	gboolean needtz = FALSE;
	icalproperty *prop;

	for (prop = icalcomponent_get_first_property (vevent, ICAL_DTEND_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (vevent, ICAL_DTEND_PROPERTY)) {
		if (_eas2ical_convert_datetime_property (prop, icaltz, isAllDayEvent,
							 icalproperty_get_dtend,
							 icalproperty_set_dtend))
			needtz = TRUE;
	}
	for (prop = icalcomponent_get_first_property (vevent, ICAL_DTSTART_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (vevent, ICAL_DTSTART_PROPERTY)) {
		if (_eas2ical_convert_datetime_property (prop, icaltz, isAllDayEvent,
							 icalproperty_get_dtstart,
							 icalproperty_set_dtstart))
			needtz = TRUE;
	}
	for (prop = icalcomponent_get_first_property (vevent, ICAL_RECURRENCEID_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (vevent, ICAL_RECURRENCEID_PROPERTY)) {
		// Must match all-day status of *parent* here!
		if (_eas2ical_convert_datetime_property (prop, icaltz, parentIsAllDayEvent,
							 _eas2cal_property_get_recurrenceid,
							 icalproperty_set_recurrenceid))
			needtz = TRUE;
	}
	for (prop = icalcomponent_get_first_property (vevent, ICAL_EXDATE_PROPERTY);
	     prop;
	     prop = icalcomponent_get_next_property (vevent, ICAL_EXDATE_PROPERTY)) {
		if (_eas2ical_convert_datetime_property (prop, icaltz, isAllDayEvent,
							 icalproperty_get_exdate,
							 icalproperty_set_exdate))
			needtz = TRUE;
	}
	if (isAllDayEvent) {
		for (prop = icalcomponent_get_first_property (vevent, ICAL_RRULE_PROPERTY);
		     prop;
		     prop = icalcomponent_get_next_property (vevent, ICAL_RRULE_PROPERTY)) {
			struct icalrecurrencetype recur = icalproperty_get_rrule (prop);
			if (!icaltime_is_null_time (recur.until)) {
				if (icaltz)
					recur.until = icaltime_convert_to_zone (recur.until,
										icaltz);
				recur.until.is_date = 1;
				icalproperty_set_rrule (prop, recur);
			}
		}
	}
	return needtz;
}

/**
 * Process the <Attendees> element during parsing of an EAS XML document
 *
 * @param  n
 *      An XML node pointing at the <Attendees> element
 * @param  vevent
 *      The VEVENT iCal component to add the parsed attendees to
 */
static void _eas2ical_process_attendees (xmlNodePtr n, icalcomponent* vevent)
{
	gchar*         value = NULL;
	icalproperty*  prop  = NULL;
	icalparameter* param = NULL;

	xmlNode* attendeeNode = NULL;

	g_debug ("Attendees element found in EAS XML");

	for (attendeeNode = n->children; attendeeNode; attendeeNode = attendeeNode->next) {
		// Variables for attendee properties and parameters
		gchar*                   email      = NULL;
		gchar*                   name       = NULL;
		icalparameter_partstat   partstat   = ICAL_PARTSTAT_NONE;	// Participatant Status parameter
		icalparameter_role       role       = ICAL_ROLE_NONE;
		xmlNodePtr               subNode    = NULL;

		g_debug ("Attendee element found in EAS XML");

		for (subNode = attendeeNode->children; subNode; subNode = subNode->next) {
			if (subNode->type == XML_ELEMENT_NODE && g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_ATTENDEE_EMAIL) == 0) {
				if (email) xmlFree (email);
				email = (gchar*) xmlNodeGetContent (subNode);
			} else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_ATTENDEE_NAME) == 0) {
				if (name) xmlFree (name);
				name = (gchar*) xmlNodeGetContent (subNode);
			} else if (subNode->type == XML_ELEMENT_NODE && g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_ATTENDEE_STATUS) == 0) {
				int status = 0;
				value = (gchar*) xmlNodeGetContent (subNode);
				status = atoi (value);
				switch (status) {
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
					g_warning ("unrecognised attendee status received");
					break;
				}// end switch status

				xmlFree (value);
				value = NULL;
			} else if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_ATTENDEE_TYPE)) {
				int roleValue = 0;
				value = (gchar*) xmlNodeGetContent (subNode);
				roleValue = atoi (value);
				switch (roleValue) {
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
					g_warning ("unrecognised attendee type received");
					break;
				}// end switch type

				xmlFree (value);
				value = NULL;
			}

		}// end for subNodes

		// Now finally build and add the property, assuming we have at least an e-mail address
		if (email && strlen (email)) {
			prop = icalproperty_new_attendee (email);

			if (email && name != NULL && strlen (name)) {
				param = icalparameter_new_cn (name);
				icalproperty_add_parameter (prop, param);
			}
			if (partstat != ICAL_PARTSTAT_NONE) {
				param = icalparameter_new_partstat (partstat);
				icalproperty_add_parameter (prop, param);
			}
			if (role != ICAL_ROLE_NONE) {
				param = icalparameter_new_role (role);
				icalproperty_add_parameter (prop, param);
			}

			icalcomponent_add_property (vevent, prop);
		}

		if (email) xmlFree (email);
		if (name) xmlFree (name);
	}
}


/**
 * Process the <Timezone> element during parsing of an EAS XML document
 *
 * @param  n
 *      An XML node pointing to the <Recurrence> element
 * @param  vtimezone
 *      The iCalendar VTIMEZONE component to add the parsed timezone properties to
 * @param  tzid
 *      Pointer to a buffer to initialise with the parsed TZID (timezone ID) string
 * @return
        icaltimezone after successful parsing or NULL in case of failure, must be freed by caller
 */
static icaltimezone* _eas2ical_process_timezone (xmlNodePtr n, icalcomponent* vtimezone, gchar** tzid)
{
	gchar*         value = NULL;
	icalproperty*  prop = NULL;
	xmlChar*       timeZoneBase64Buffer = xmlNodeGetContent (n);
	gsize          timeZoneRawBytesSize = 0;
	guchar*        timeZoneRawBytes = g_base64_decode ( (const gchar*) timeZoneBase64Buffer, &timeZoneRawBytesSize);
	EasTimeZone    timeZoneStruct;
	icaltimezone*  icaltz = NULL;

	// TODO Check decode of timezone for endianess problems

	if (timeZoneRawBytesSize == sizeof (EasTimeZone)) {
		memcpy (&timeZoneStruct, timeZoneRawBytes, timeZoneRawBytesSize);
		g_free (timeZoneRawBytes);
		timeZoneRawBytes = NULL;

		{
			char *standard = g_utf16_to_utf8 ( (const gunichar2*) timeZoneStruct.StandardName,
							   (sizeof (timeZoneStruct.StandardName) / sizeof (guint16)), NULL, NULL, NULL);
			char *daylight = g_utf16_to_utf8 ( (const gunichar2*) timeZoneStruct.DaylightName,
							   (sizeof (timeZoneStruct.DaylightName) / sizeof (guint16)), NULL, NULL, NULL);
			g_debug ("process timezone %s => bias %d, standard bias %d, daylight bias %d, standard '%s', daylight '%s'",
				 timeZoneBase64Buffer,
				 timeZoneStruct.Bias,
				 timeZoneStruct.StandardBias,
				 timeZoneStruct.DaylightBias,
				 standard, daylight);
			g_free (standard);
			g_free (daylight);
		}

		{
			// Calculate the timezone offsets. See _ical2eas_process_xstandard_xdaylight()
			// comments for a full explanation of how EAS Bias relates to iCal UTC offsets
			const gint32                standardUtcOffsetMins = -1 * (timeZoneStruct.Bias + timeZoneStruct.StandardBias);
			const gint32                daylightUtcOffsetMins = -1 * (timeZoneStruct.Bias + timeZoneStruct.DaylightBias);
			icalcomponent*              xstandard             = NULL;
			icalcomponent*              xdaylight             = NULL;
			struct icalrecurrencetype   rrule;
			struct icaltimetype         time;

			if (standardUtcOffsetMins == 0 &&
			    daylightUtcOffsetMins == 0) {
				// is UTC time zone, ignore it
				goto done;
			}

			// Using StandardName as the TZID
			// (Doesn't matter if it's not an exact description: this field is only used internally
			// during iCalendar encoding/decoding)
			// Note: using tzid here rather than value, as we need it elsewhere in this function
			*tzid = g_utf16_to_utf8 ( (const gunichar2*) timeZoneStruct.StandardName,
						  (sizeof (timeZoneStruct.StandardName) / sizeof (guint16)), NULL, NULL, NULL);
			// If no StandardName was supplied, we can just use a temporary name instead.
			// No need to support more than one: the EAS calendar will only have one Timezone
			// element. And no need to localise, as it's only used internally.
			if (*tzid == NULL || strlen (*tzid) == 0) {
				g_free (*tzid);
				*tzid = g_strdup (ICAL_DEFAULT_TZID);
			}
			prop = icalproperty_new_tzid (*tzid);
			icalcomponent_add_property (vtimezone, prop);


			//
			// STANDARD component
			//

			xstandard = icalcomponent_new (ICAL_XSTANDARD_COMPONENT);

			icalrecurrencetype_clear (&rrule);

			// If timeZoneStruct.StandardDate.Year == 0 we need to convert it into
			// the start date of a recurring sequence, and add an RRULE.
			if (timeZoneStruct.StandardDate.Year == 0 &&
			    timeZoneStruct.StandardDate.Month != 0) {
				_eas2ical_convert_relative_timezone_date (&timeZoneStruct.StandardDate, &rrule);
			}

			// Add the DTSTART property
			time = icaltime_null_time();
			time.year = timeZoneStruct.StandardDate.Year;
			time.month = timeZoneStruct.StandardDate.Month;
			time.day = timeZoneStruct.StandardDate.Day;
			time.hour = timeZoneStruct.StandardDate.Hour;
			time.minute = timeZoneStruct.StandardDate.Minute;
			time.second = timeZoneStruct.StandardDate.Second;
			prop = icalproperty_new_dtstart (time);
			icalcomponent_add_property (xstandard, prop);

			// Add the RRULE (if required)
			if (rrule.freq != ICAL_NO_RECURRENCE) {
				prop = icalproperty_new_rrule (rrule);
				icalcomponent_add_property (xstandard, prop);
			}

			// Add TZOFFSETFROM and TZOFFSETTO
			// Note that libical expects these properties in seconds.
			prop = icalproperty_new_tzoffsetfrom (daylightUtcOffsetMins * SECONDS_PER_MINUTE);
			icalcomponent_add_property (xstandard, prop);
			prop = icalproperty_new_tzoffsetto (standardUtcOffsetMins * SECONDS_PER_MINUTE);
			icalcomponent_add_property (xstandard, prop);

			value = g_utf16_to_utf8 ( (const gunichar2*) timeZoneStruct.StandardName, (sizeof (timeZoneStruct.StandardName) / sizeof (guint16)), NULL, NULL, NULL);
			if (value) {
				if (strlen (value)) {
					prop = icalproperty_new_tzname (value);
					icalcomponent_add_property (xstandard, prop);
				}
				g_free (value);
				value = NULL;
			}

			// And now add the STANDARD component to the VTIMEZONE
			icalcomponent_add_component (vtimezone, xstandard);


			//
			// DAYLIGHT component
			//
			// FIXME: How does it indicate that the daylight zone doesn't
			//        really exist?
			if (timeZoneStruct.DaylightDate.Month != 0) {

				xdaylight = icalcomponent_new (ICAL_XDAYLIGHT_COMPONENT);

				// Reset the RRULE
				icalrecurrencetype_clear (&rrule);

				// If timeZoneStruct.DaylightDate.Year == 0 we need to convert it into
				// the start date of a recurring sequence, and add an RRULE.
				if (timeZoneStruct.DaylightDate.Year == 0) {
					_eas2ical_convert_relative_timezone_date (&timeZoneStruct.DaylightDate, &rrule);
				}

				// Add the DTSTART property
				time = icaltime_null_time();
				time.year = timeZoneStruct.DaylightDate.Year;
				time.month = timeZoneStruct.DaylightDate.Month;
				time.day = timeZoneStruct.DaylightDate.Day;
				time.hour = timeZoneStruct.DaylightDate.Hour;
				time.minute = timeZoneStruct.DaylightDate.Minute;
				time.second = timeZoneStruct.DaylightDate.Second;
				prop = icalproperty_new_dtstart (time);
				icalcomponent_add_property (xdaylight, prop);

				// Add the RRULE (if required)
				if (rrule.freq != ICAL_NO_RECURRENCE) {
					prop = icalproperty_new_rrule (rrule);
					icalcomponent_add_property (xdaylight, prop);
				}

				// Add TZOFFSETFROM and TZOFFSETTO
				// Note that libical expects these properties in seconds.
				prop = icalproperty_new_tzoffsetfrom (standardUtcOffsetMins * SECONDS_PER_MINUTE);
				icalcomponent_add_property (xdaylight, prop);
				prop = icalproperty_new_tzoffsetto (daylightUtcOffsetMins * SECONDS_PER_MINUTE);
				icalcomponent_add_property (xdaylight, prop);

				value = g_utf16_to_utf8 ( (const gunichar2*) timeZoneStruct.DaylightName, (sizeof (timeZoneStruct.DaylightName) / sizeof (guint16)), NULL, NULL, NULL);
				if (value) {
					if (strlen (value)) {
						prop = icalproperty_new_tzname (value);
						icalcomponent_add_property (xdaylight, prop);
					}
					g_free (value);
					value = NULL;
				}

				// And now add the DAYLIGHT component to the VTIMEZONE
				icalcomponent_add_component (vtimezone, xdaylight);
			}
		}

		icaltz = icaltimezone_new();
		if (icaltz)
			icaltimezone_set_component(icaltz, vtimezone);
	} // timeZoneRawBytesSize == sizeof(timeZoneStruct)
	else {
		g_critical ("TimeZone BLOB did not match sizeof(EasTimeZone)");
	}

 done:
	xmlFree (timeZoneBase64Buffer);
	return icaltz;
}


/**
 * Process the <Recurrence> element during parsing of an EAS XML document
 *
 * @param  n
 *      An XML node pointing to the <Recurrence> element
 * @param  vevent
 *      The iCalendar VEVENT component to add the parsed RRULE property to
 */
static void _eas2ical_process_recurrence (xmlNodePtr n, icalcomponent* vevent)
{
	gchar*         value    = NULL;
	icalproperty*  prop     = NULL;
	xmlNode*       subNode  = NULL;
	struct icalrecurrencetype recur;

	// Ensure the icalrecurrencetype is null
	icalrecurrencetype_clear (&recur);

	g_debug ("Recurrence element found in EAS XML");

	for (subNode = n->children; subNode; subNode = subNode->next) {
		const gchar* elemName = (const gchar*) subNode->name;
		value = (gchar*) xmlNodeGetContent (subNode);


		// nothing to do
		if (subNode->type != XML_ELEMENT_NODE) {
		}

		// Type
		else if (g_strcmp0 (elemName, EAS_ELEMENT_TYPE) == 0) {
			int typeInt = atoi (value);
			switch (typeInt) {
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
		else if (g_strcmp0 (elemName, EAS_ELEMENT_OCCURRENCES) == 0) {
			// From [MS-ASCAL]:
			// The Occurrences element and the Until element (section 2.2.2.18.7) are mutually exclusive. It is
			// recommended that only one of these elements be included in a Recurrence element (section
			// 2.2.2.18) in a Sync command request. If both elements are included, then the server MUST respect
			// the value of the Occurrences element and ignore the Until element.
			recur.count = atoi (value);
			recur.until = icaltime_null_time();
		}

		// Interval
		else if (g_strcmp0 (elemName, EAS_ELEMENT_INTERVAL) == 0) {
			recur.interval = (short) atoi (value);
		}

		// WeekOfMonth
		else if (g_strcmp0 (elemName, EAS_ELEMENT_WEEKOFMONTH) == 0) {
			g_warning ("DATA LOSS: Cannot handle <Calendar><Recurrence><WeekOfMonth> element (value=%d)", atoi (value));
		}

		// DayOfWeek
		else if (g_strcmp0 (elemName, EAS_ELEMENT_DAYOFWEEK) == 0) {
			// A recurrence rule can target any combination of the 7 week days.
			// EAS encodes these as a bit set in a single int value.
			// libical encodes them as an array (max size 7) of icalrecurrencetype_weekday
			// enum values.
			// This block of code converts the former into the latter.

			int dayOfWeek = atoi (value);
			int index = 0;

			// Note: must use IF for subsequent blocks, not ELSE IF
			if (dayOfWeek & DAY_OF_WEEK_MONDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_MONDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_TUESDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_TUESDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_WEDNESDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_WEDNESDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_THURSDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_THURSDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_FRIDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_FRIDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_SATURDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_SATURDAY_WEEKDAY, 0);
			}
			if (dayOfWeek & DAY_OF_WEEK_SUNDAY) {
				recur.by_day[index++] = _eas2ical_make_rrule_by_day_value (ICAL_SUNDAY_WEEKDAY, 0);
			}
		}

		// MonthOfYear
		else if (g_strcmp0 (elemName, EAS_ELEMENT_MONTHOFYEAR) == 0) {
			recur.by_month[0] = (short) atoi (value);
		}

		// Until
		else if (g_strcmp0 (elemName, EAS_ELEMENT_UNTIL) == 0) {
			// From [MS-ASCAL]:
			// The Occurrences element and the Until element (section 2.2.2.18.7) are mutually exclusive. It is
			// recommended that only one of these elements be included in a Recurrence element (section
			// 2.2.2.18) in a Sync command request. If both elements are included, then the server MUST respect
			// the value of the Occurrences element and ignore the Until element.
			if (recur.count == 0) {
				// fix inclusive vs. exclusive difference by subtracting one second
				recur.until = icaltime_from_string (value);
			}
		}

		// DayOfMonth
		else if (g_strcmp0 (elemName, EAS_ELEMENT_DAYOFMONTH) == 0) {
			recur.by_month_day[0] = (short) atoi (value);
		}

		// CalendarType
		else if (g_strcmp0 (elemName, EAS_ELEMENT_CALENDARTYPE) == 0) {
			const int calType = atoi (value);

			if ( (calType != 0)   // Default
			     &&
			     (calType != 1)) { // Gregorian
				// No way of handling in iCal
				g_warning ("DATA LOSS: Encountered a calendar type we can't handle in the <Calendar><Recurrence><CalendarType> element: %d", calType);
			}
		}

		// IsLeapMonth
		else if (g_strcmp0 (elemName, EAS_ELEMENT_ISLEAPMONTH) == 0) {
			if (atoi (value) == 1) {
				// This has nothing to do with Gregorian calendar leap years (ie. 29 days in Feb).
				// It only applies to non-Gregorian calendars so can't be handled in iCal.
				g_warning ("DATA LOSS: Cannot handle <Calendar><Recurrence><IsLeapMonth> element");
			}
		}

		// FirstDayOfWeek
		else if (g_strcmp0 (elemName, EAS_ELEMENT_FIRSTDAYOFWEEK) == 0) {
			int firstDayOfWeek = atoi (value);

			// EAS value is in range 0=Sunday..6=Saturday
			// iCal value is in range 0=NoWeekday, 1=Sunday..7=Saturday
			recur.week_start = (icalrecurrencetype_weekday) firstDayOfWeek + 1;
		}

		// Other fields...
		else {
			g_warning ("DATA LOSS: Unknown element encountered in <Recurrence> element: %s", elemName);
		}

		xmlFree (value);
	}

	prop = icalproperty_new_rrule (recur);
	icalcomponent_add_property (vevent, prop);
}


/**
 * Process the <Exceptions> element during parsing of an EAS XML document
 *
 * @param  n
 *      An XML node pointing to the <Exceptions> element
 * @param  vevent
 *      The iCalendar VEVENT component to add the parsed items property to
 */
static GSList* _eas2ical_process_exceptions (xmlNodePtr n, icalcomponent* vevent)
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

	g_debug ("Exceptions element found in EAS XML");

	// Iterate through the <Exception> elements
	for (exceptionNode = n->children; exceptionNode; exceptionNode = exceptionNode->next) {
		GHashTable* newEventValues = NULL;
		gchar* exceptionStartTime = NULL;
		gboolean deleted = FALSE;

		if (exceptionNode->type != XML_ELEMENT_NODE)
			continue;

		// Iterate through each Exception's properties
		for (subNode = exceptionNode->children; subNode; subNode = subNode->next) {
			const gchar* name;

			if (subNode->type != XML_ELEMENT_NODE)
				continue;

			name = (const gchar*) subNode->name;
			value = (gchar*) xmlNodeGetContent (subNode);

			if (g_strcmp0 (name, EAS_ELEMENT_DELETED) == 0) {
				deleted = (g_strcmp0 (value, EAS_BOOLEAN_TRUE) == 0);
			} else if (g_strcmp0 (name, EAS_ELEMENT_EXCEPTIONSTARTTIME) == 0) {
				exceptionStartTime = g_strdup (value);
			}
			// only content of <Data> present in <Body> is required to be stored in hashTable
			else if (g_strcmp0 (name, EAS_ELEMENT_BODY) == 0) {
				xmlNodePtr bodySubNode = NULL;
				for (bodySubNode = subNode->children; bodySubNode; bodySubNode = bodySubNode->next) {
					if (bodySubNode->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) bodySubNode->name, EAS_ELEMENT_DATA)) {
						if (value) xmlFree (value);
						value = (gchar*) xmlNodeGetContent (bodySubNode);
						if (newEventValues == NULL) {
							newEventValues = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
						}
						g_hash_table_insert (newEventValues, g_strdup (name), g_strdup (value));
						break;
					}
				}
			}
			// contents of all <Category> present in <Categories> is required to be stored in hashTable
			else if (g_strcmp0 (name, EAS_ELEMENT_CATEGORIES) == 0) {
				xmlNodePtr catNode = NULL;
				GString* categories = g_string_new ("");
				int categoryLength, index = 0;
				for (catNode = subNode->children; catNode; catNode = catNode->next) {
					if (catNode->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) catNode->name, EAS_ELEMENT_CATEGORY)) {
						if (value) xmlFree (value);
						value = (gchar*) xmlNodeGetContent (catNode);
						for (index = 0, categoryLength = strlen (value); index < categoryLength; index++) {
							if (value[index] == ',')
								categories = g_string_append (categories, "/,");
							else
								categories = g_string_append_c (categories, value[index]);
						}
						categories = g_string_append_c (categories, ',');
					}

				}
				categories = g_string_erase (categories, categories->len - 1, -1);
				if (newEventValues == NULL) {
					newEventValues = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
				}
				g_hash_table_insert (newEventValues, g_strdup (name), g_string_free (categories, FALSE));
			} else if (strlen (value) > 0) {
				// We've got a non-trivial exception that will
				// require adding a new event: build a hash of its values
				// and add to the list
				if (newEventValues == NULL) {
					newEventValues = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
				}
				g_hash_table_insert (newEventValues, g_strdup (name), g_strdup (value));
			}

			xmlFree (value);
		}

		// ExceptionStartTime is mandatory so check we've got one
		if (exceptionStartTime == NULL) {
			g_warning ("DATA LOSS: <Exception> element found with no ExceptionStartTime.");
			continue;
		}

		// If the <Deleted> value is set to 1, it's dead easy: it maps straight onto an
		// EXDATE an we can ignore the other elements.
		if (deleted) {
			// Add an EXDATE property

			// I'm ASSUMING here that we add multiple single-value EXDATE properties and
			// libical takes care of merging them into one (just as it splits them when
			// we're reading an iCal). TODO: check this during testing...
			icalproperty* exdate = icalproperty_new_exdate (icaltime_from_string (exceptionStartTime));
			icalcomponent_add_property (vevent, exdate); // vevent takes ownership of exdate
		}
		// If it's not deleted, but the only other element present is ExceptionStartTime,
		// then it's an RDATE (i.e. just a one-off recurrence of the same event but not
		// included in the regular recurrence sequence)
		else if (newEventValues == NULL) {
			icalproperty* rdate = NULL;

			// Same assumption as for EXDATE: that we just add multiple properties
			// and libical takes care of merging them. TODO: check this during testing...
			struct icaldatetimeperiodtype dtper;
			dtper.period = icalperiodtype_null_period();
			dtper.time = icaltime_from_string (exceptionStartTime);
			rdate = icalproperty_new_rdate (dtper);
			icalcomponent_add_property (vevent, rdate); // vevent takes ownership of rdate
		}
		// Otherwise it's neither an EXDATE or an RDATE: it's a new instance of the
		// event with more substantial changes (e.g. start time/end time/subject/etc.
		// has changed)
		else {
			// Add ExceptionStartTime to the hash now too
			g_hash_table_insert (newEventValues, g_strdup (EAS_ELEMENT_EXCEPTIONSTARTTIME), g_strdup (exceptionStartTime));

			// And add the hash table to the list to be returned
			// (Using prepend rather than append for efficiency)
			// (No need to new the list first, it's allocated during prepend:
			// http://developer.gnome.org/glib/stable/glib-Singly-Linked-Lists.html#g-slist-alloc)
			listOfNewEventEvents = g_slist_prepend (listOfNewEventEvents, newEventValues);
		}

		g_free (exceptionStartTime);
	}// end of for loop

	return listOfNewEventEvents;
}


/**
 * Add additional VEVENT components to the VCALENDAR for non-trivial recurrence exceptions.
 *
 * EAS supports non-trivial exceptions to a recurrence rule (i.e. just just deleted recurrences,
 * but recurrences where field values have changed, such as subject or start time/end time).
 * The only way we can support these in iCalendar is to create them as additional VEVENTS.
 * So we gather all the properties of these exceptions into a list during parsing (see
 * _eas2ical_process_exceptions()) then pass thm to this function which converts them into
 * VEVENTS.
 *
 * We try to maintain the link to the original VEVENT in the UID field: each of the new "child"
 * VEVENTS has the original VEVENT's UID with the exception's start time appended, e.g.
 *
 *   Original VEVENT UID: 0123456789ABCDEF
 *   Child VEVENT UIDs:   0123456789ABCDEF_20110102T103000Z
 *                        0123456789ABCDEF_20110103T103000Z
 *                        etc.
 *
 * TODO: look for this format of UID when parsing VEVENTS to try and match them up again.
 *
 * @param  vcalendar
 *      The outer VCALENDAR component which owns the "parent" VEVENT (and into which
 *      we will add the new "child" VEVENTs)
 * @param  vevent
 *      The "parent" VEVENT, fully converted from EAS XML format
 * @param  exceptionEvents
 *      A list of hash tables, each containing the changed field values for a single exception
 *
 * @return
 *      TRUE if time zone defition is needed
 */
static gboolean _eas2ical_add_exception_events (icalcomponent* vcalendar,
						icalcomponent* vevent,
						GSList* exceptionEvents,
						icaltimezone *icaltz,
						gboolean parentIsAllDayEvent)
{
	gboolean needtz = FALSE;

	if (vcalendar && vevent && exceptionEvents) {
		const guint newEventCount = g_slist_length (exceptionEvents);
		guint index = 0;

		// Iterate through the list adding each exception event in turn
		for (index = 0; index < newEventCount; index++) {
			icalcomponent* newEvent = icalcomponent_new_clone (vevent);
			GHashTable* exceptionProperties = (GHashTable*) g_slist_nth_data (exceptionEvents, index);
			icalproperty* prop = NULL;
			gchar* value = NULL;
			gboolean newIsAllDayEvent = parentIsAllDayEvent;

			if (exceptionProperties == NULL) {
				g_warning ("_eas2ical_add_exception_events(): NULL hash table found in exceptionEvents.");
				break;
			}

			// Remove any recurrence (RRULE, RDATE and EXDATE) properies from the new event
			while ( (prop = icalcomponent_get_first_property (newEvent, ICAL_RRULE_PROPERTY)) != NULL) {
				icalcomponent_remove_property (newEvent, prop);
				icalproperty_free (prop);
				prop = NULL;
			}
			while ( (prop = icalcomponent_get_first_property (newEvent, ICAL_RDATE_PROPERTY)) != NULL) {
				icalcomponent_remove_property (newEvent, prop);
				icalproperty_free (prop);
				prop = NULL;
			}
			while ( (prop = icalcomponent_get_first_property (newEvent, ICAL_EXDATE_PROPERTY)) != NULL) {
				icalcomponent_remove_property (newEvent, prop);
				icalproperty_free (prop);
				prop = NULL;
			}

			// Form a new UID for the new event as follows:
			// {Original event UID}_{Exception start time}
			prop = icalcomponent_get_first_property (newEvent, ICAL_UID_PROPERTY); // Retains ownership of the pointer
			icalproperty_set_uid (prop, (const gchar*) icalproperty_get_uid (prop));
			prop = NULL;

			// TODO: as we're parsing these from the <Exceptions> element, I think we need to
			// add an EXDATE for this ExceptionStartTime. I think ExceptionStartTime identifies
			// the recurrence this is *replacing*: StartTime specifies this exception's own
			// start time.


			// Add the other properties from the hash

			// StartTime
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_STARTTIME)) != NULL) {
				// If this exception has a new start time, use that. Otherwise we default
				// to the start time of the original event (as per [MS-ASCAL]).
				// TODO: this needs testing: [MS-ASCAL] is a bit ambiguous around
				// <StartTime> vs. <ExceptionStartTime>
				struct icaltimetype dateTime;

				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_DTSTART_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				dateTime = icaltime_from_string (value);
				prop = icalproperty_new_dtstart (dateTime);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}

			// EndTime
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_ENDTIME)) != NULL) {
				struct icaltimetype dateTime;

				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_DTEND_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				dateTime = icaltime_from_string (value);
				prop = icalproperty_new_dtend (dateTime);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}

			// Subject
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_SUBJECT)) != NULL) {
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_SUMMARY_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_summary (value);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}

			// Location
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_LOCATION)) != NULL) {
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_LOCATION_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_location (value);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}

			// Categories
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_CATEGORIES)) != NULL) {
				GString* category = g_string_new ("");
				int index, categoriesLength;
				for (index = 0, categoriesLength = strlen (value); index < categoriesLength; index++) {
					if (value[index] == '/' && index + 1 != categoriesLength && value[index + 1] == ',') {
						// ignore
					} else if (value[index] == ',' && index != 0 && value[index - 1] == '/')
						category = g_string_append_c (category, ',');
					else if (value[index] == ',' && index != 0 && value[index - 1] != '/') {


						prop = icalproperty_new_categories (category->str);
						icalcomponent_add_property (newEvent, prop); // vevent takes ownership
						g_string_free (category, TRUE);
						category = g_string_new ("");

					} else
						category = g_string_append_c (category, index[value]);

				}

				prop = icalproperty_new_categories (category->str);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
				g_string_free (category, TRUE);
			}

			// Sensitivity
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_SENSITIVITY)) != NULL) {
				// Clear out any existing property
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_CLASS_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_class (_eas2ical_convert_sensitivity_to_class (value));
				icalcomponent_add_property (newEvent, prop);
			}

			// BusyStatus
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_BUSYSTATUS)) != NULL) {
				// Clear out any existing property
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_TRANSP_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_transp (_eas2ical_convert_busystatus_to_transp (value));
				icalcomponent_add_property (newEvent, prop);
			}

			// AllDayEvent
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_ALLDAYEVENT)) != NULL) {
				// TODO
			}

			// Reminder
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_REMINDER)) != NULL) {
				icalcomponent* valarm = NULL;
				struct icaltriggertype trigger;

				// Remove any existing VALARM
				if ( (valarm = icalcomponent_get_first_component (newEvent, ICAL_VALARM_COMPONENT)) != NULL) {
					icalcomponent_remove_component (newEvent, valarm); // We now have ownership of valarm
					icalcomponent_free (valarm);
					valarm = NULL;
				}

				valarm = icalcomponent_new (ICAL_VALARM_COMPONENT);

				// TODO: find a way of merging this with the other VALARM creation
				// code to avoid the duplication

				// Build an icaltriggertype structure
				trigger = icaltriggertype_from_int (0); // Null the fields first
				trigger.duration.is_neg = 1;
				trigger.duration.minutes = (unsigned int) atoi (value);

				prop = icalproperty_new_action (ICAL_ACTION_DISPLAY);
				icalcomponent_add_property (valarm, prop);

				prop = icalproperty_new_description (ICAL_DEFAULT_REMINDER_NAME); // TODO: make this configurable
				icalcomponent_add_property (valarm, prop);

				prop = icalproperty_new_trigger (trigger);
				icalcomponent_add_property (valarm, prop);

				icalcomponent_add_component (newEvent, valarm);
			}

			// DtStamp
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_DTSTAMP)) != NULL) {
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_DTSTAMP_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_dtstamp (icaltime_from_string (value));
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}

			// ExceptionStartTime -> convert to RecurrenceID
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_EXCEPTIONSTARTTIME)) != NULL) {
				prop = icalproperty_new_recurrenceid(icaltime_from_string (value));
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}
			
			// MeetingStatus
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_MEETINGSTATUS)) != NULL) {
				// TODO
			}

			// AppointmentReplyTime
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_APPOINTMENTREPLYTIME)) != NULL) {
				// TODO
			}

			// ResponseType
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_RESPONSETYPE)) != NULL) {
				// TODO
			}

			// Body
			if ( (value = (gchar*) g_hash_table_lookup (exceptionProperties, EAS_ELEMENT_BODY)) != NULL) {
				if ( (prop = icalcomponent_get_first_property (newEvent, ICAL_DESCRIPTION_PROPERTY)) != NULL) {
					icalcomponent_remove_property (newEvent, prop); // Now we have ownership of prop
					icalproperty_free (prop);
					prop = NULL;
				}
				prop = icalproperty_new_description (value);
				icalcomponent_add_property (newEvent, prop); // vevent takes ownership
			}


			// Finally, destroy the hash and replace the list entry with NULL
			g_hash_table_destroy (exceptionProperties);
			g_slist_nth (exceptionEvents, index)->data = NULL;

			if (_eas2ical_convert_component (newEvent, icaltz, newIsAllDayEvent, parentIsAllDayEvent))
				needtz = TRUE;

			// Add the new event to the parent VCALENDAR
			icalcomponent_add_component (vcalendar, newEvent);

			// Debug output
			g_debug ("Added new exception VEVENT to the VCALENDAR:n%s", icalcomponent_as_ical_string (newEvent));
		}
	}

	return needtz;
}



/**
 * Parse an XML-formatted calendar object received from ActiveSync and return
 * it as a serialised iCalendar object.
 *
 * In the first pass, the XML representation is copied into:
 * - an icalcomponent with little to no translations (in particular, all time stamps in
 *   the original UTC date-time format)
 * - meta-information (all-day flag, icaltimezone)
 * - list of detached recurrences
 *
 * Then once the time zone and all-day flag are known, the UTC time stamps
 * are adapted:
 * - DTSTART/END/RECURRENCE-ID converted to zone,
 *   stripped down to date-only, TZID added if needed
 * - if all-day, RRULE UNTIL is converted to zone and converted to date-only
 *   (otherwise it remains as UTC)
 *
 * @param  node
 *      ActiveSync XML <ApplicationData> object containing a calendar.
 * @param  server_id
 *      The ActiveSync server ID from the response
 */
gchar* eas_cal_info_translator_parse_response (xmlNodePtr node, gchar* server_id)
{
	// Variable for the return value
	gchar* result = NULL;

	// Variable to store the TZID value when decoding a <calendar:Timezone> element
	// so we can use it in the rest of the iCal's date/time fields.
	gchar* tzid   = NULL;

	// iCalendar objects
	struct icaltimetype dateTime;
	struct icaltriggertype trigger;


	xmlNodePtr n = node;

	EasItemInfo* calInfo = NULL;

	// iCalendar objects
	icalcomponent* vcalendar = icalcomponent_new (ICAL_VCALENDAR_COMPONENT);
	icalcomponent* vevent = icalcomponent_new (ICAL_VEVENT_COMPONENT);
	icalcomponent* valarm = icalcomponent_new (ICAL_VALARM_COMPONENT);
	icalcomponent* vtimezone = icalcomponent_new (ICAL_VTIMEZONE_COMPONENT);
	icaltimezone* icaltz = NULL;
	icalproperty* prop = NULL;
	icalparameter* param = NULL;
	gboolean isAllDayEvent = FALSE;
	gboolean needtz = FALSE;
	gchar* organizerName = NULL;
	gchar* organizerEmail = NULL;
	GSList* newExceptionEvents = NULL;

	// TODO: get all these strings into constants/#defines

	// TODO: make the PRODID configurable somehow
	prop = icalproperty_new_prodid (ICAL_PROPERTY_PRODID);
	icalcomponent_add_property (vcalendar, prop);

	prop = icalproperty_new_version (ICAL_PROPERTY_VERSION);
	icalcomponent_add_property (vcalendar, prop);

	prop = icalproperty_new_method (ICAL_METHOD_PUBLISH);
	icalcomponent_add_property (vcalendar, prop);

	for (n = n->children; n; n = n->next) {
		if (n->type == XML_ELEMENT_NODE) {
			const gchar* name = (const gchar*) (n->name);
			gchar* value  = NULL;

			//
			// Subject
			//
			if (g_strcmp0 (name, EAS_ELEMENT_SUBJECT) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new_summary (value);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// StartTime
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_STARTTIME) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				dateTime = icaltime_from_string (value);
				prop = icalproperty_new_dtstart (dateTime);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// EndTime
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_ENDTIME) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				dateTime = icaltime_from_string (value);
				prop = icalproperty_new_dtend (dateTime);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// DtStamp
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_DTSTAMP) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				dateTime = icaltime_from_string (value);
				prop = icalproperty_new_dtstamp (dateTime);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// UID
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_UID) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new_uid (value);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// Location
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_LOCATION) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new_location (value);
				icalcomponent_add_property (vevent, prop);
			}

			//
			// Body
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_BODY) == 0) {
				xmlNodePtr subNode = NULL;
				for (subNode = n->children; subNode; subNode = subNode->next) {
					if (subNode->type == XML_ELEMENT_NODE && !g_strcmp0 ( (gchar*) subNode->name, EAS_ELEMENT_DATA)) {
						if (value) xmlFree (value);
						value = (gchar*) xmlNodeGetContent (subNode);
						prop = icalproperty_new_description (value);
						icalcomponent_add_property (vevent, prop);
						break;
					}
				}
			}

			//
			// Sensitivity
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_SENSITIVITY) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new_class (_eas2ical_convert_sensitivity_to_class (value));
				icalcomponent_add_property (vevent, prop);
			}

			//
			// BusyStatus
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_BUSYSTATUS) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new_transp (_eas2ical_convert_busystatus_to_transp (value));
				icalcomponent_add_property (vevent, prop);
			}

			//
			// Categories
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_CATEGORIES) == 0) {
				xmlNode* catNode = NULL;
				for (catNode = n->children; catNode; catNode = catNode->next) {
					if (catNode->type != XML_ELEMENT_NODE)
						continue;

					if (value) xmlFree (value);
					value = (gchar*) xmlNodeGetContent (catNode);
					prop = icalproperty_new_categories (value);
					icalcomponent_add_property (vevent, prop);
				}
			}

			//
			// Reminder
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_REMINDER) == 0) {
				value = (gchar*) xmlNodeGetContent (n);

				// Build an icaltriggertype structure
				trigger = icaltriggertype_from_int (0); // Null the fields first
				trigger.duration.is_neg = 1;
				trigger.duration.minutes = (unsigned int) atoi (value);

				prop = icalproperty_new_action (ICAL_ACTION_DISPLAY);
				icalcomponent_add_property (valarm, prop);

				prop = icalproperty_new_description (ICAL_DEFAULT_REMINDER_NAME); // TODO: make this configurable
				icalcomponent_add_property (valarm, prop);

				prop = icalproperty_new_trigger (trigger);
				icalcomponent_add_property (valarm, prop);
			}

			//
			// AllDayEvent
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_ALLDAYEVENT) == 0) {
				value = (gchar*) xmlNodeGetContent (n);
				isAllDayEvent = atoi (value) == 1;
			}

			//
			// OrganizerName
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_ORGANIZER_NAME) == 0) {
				organizerName = (gchar*) xmlNodeGetContent (n);
				// That's all for now: deal with it after the loop completes so we
				// have both OrganizerName and OrganizerEmail
			}

			//
			// OrganizerEmail
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_ORGANIZER_EMAIL) == 0) {
				organizerEmail = (gchar*) xmlNodeGetContent (n);
				// That's all for now: deal with it after the loop completes so we
				// have both OrganizerName and OrganizerEmail
			}

			//
			// Attendees
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_ATTENDEES) == 0) {
				_eas2ical_process_attendees (n, vevent);
			}

			//
			// TimeZone
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_TIMEZONE) == 0) {
				icaltz = _eas2ical_process_timezone (n, vtimezone, &tzid);
			}

			//
			// Recurrence
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_RECURRENCE) == 0) {
				_eas2ical_process_recurrence (n, vevent);
			}

			//
			// Exceptions
			//
			else if (g_strcmp0 (name, EAS_ELEMENT_EXCEPTIONS) == 0) {
				newExceptionEvents = _eas2ical_process_exceptions (n, vevent);
				// This is dealt with below...
			}

			//
			// Unmapped data fields
			//
			else {
				// Build a new custom property called X-MEEGO-ACTIVESYNCD-{ElementName}
				gchar* propertyName = g_strconcat (ICAL_EXTENSION_PROPERTY_PREFIX, name, NULL);
				value = (gchar*) xmlNodeGetContent (n);
				prop = icalproperty_new (ICAL_X_PROPERTY);

				g_debug ("Found EAS element that doesn't map to a VEVENT property. Creating X property %s:%s", propertyName, value);

				icalproperty_set_x_name (prop, propertyName);
				icalproperty_set_value (prop, icalvalue_new_from_string (ICAL_X_VALUE, value));
				icalcomponent_add_property (vevent, prop);
				g_free (propertyName);
				propertyName = NULL;
			}

			if (value) xmlFree (value);
			value = NULL;
		}
	}

	// Deal with OrganizerName and OrganizerEmail
	if (organizerEmail) {
		prop = icalproperty_new_organizer (organizerEmail);
		xmlFree (organizerEmail);
		organizerEmail = NULL;

		if (organizerName) {
			param = icalparameter_new_cn (organizerName);
			icalproperty_add_parameter (prop, param);
			xmlFree (organizerName);
			organizerName = NULL;
		}

		icalcomponent_add_property (vevent, prop);
	}

	// Check organizerName again, so we free it if we had a name but no e-mail
	if (organizerName) {
		// TODO: Is there any way we can use the name without the e-mail address?
		g_warning ("OrganizerName element found but no OrganizerEmail");
		xmlFree (organizerName);
		organizerName = NULL;
	}

	// fix parent event
	if (_eas2ical_convert_component (vevent, icaltz, isAllDayEvent, isAllDayEvent))
		needtz = TRUE;

	// Add the subcomponents to their parent components
	icalcomponent_add_component (vcalendar, vevent);
	if (icalcomponent_count_properties (valarm, ICAL_ANY_PROPERTY) > 0) {
		icalcomponent_add_component (vevent, valarm);
	}

	// Now handle any non-trivial exception events we found in the <Exceptions> element
	if (newExceptionEvents) {
		_eas2ical_add_exception_events (vcalendar, vevent, newExceptionEvents, icaltz, isAllDayEvent);
		// _eas2ical_add_exception_events() destroys the hash tables as it goes
		// so all we need to do here is free the list
		g_slist_free (newExceptionEvents);

		// remove synthesized parent (see eas_cal_info_translator_parse_request())
		if (!g_strcmp0 (icalcomponent_get_summary (vevent), ACTIVESYNCD_PSEUDO_EVENT) &&
		    g_getenv ("EAS_DEBUG_DETACHED_RECURRENCES") == NULL) {
			icalcomponent_remove_component (vcalendar, vevent);
			icalcomponent_free (vevent);
			vevent = NULL;
		}
	}

	if (needtz)
		icalcomponent_add_component (vcalendar, icalcomponent_new_clone (vtimezone));

	// Now insert the server ID and iCalendar into an EasCalInfo object and serialise it
	calInfo = eas_item_info_new();
	calInfo->data = (gchar*) icalcomponent_as_ical_string_r (vcalendar); // Ownership passes to the EasCalInfo
	calInfo->server_id = (gchar*) server_id;
	if (!eas_item_info_serialise (calInfo, &result)) {
		// TODO: log error
		result = NULL;
	}

	// Free the EasCalInfo GObject
	g_object_unref (calInfo);

	// Free the libical components
	// (It's not clear if freeing a component also frees its children, but in any case
	// some of these (e.g. vtimezone & valarm) won't have been added as children if they
	// weren't present in the XML.)
	// Note: the libical examples show that a property doesn't have to be freed once added to a component
	icalcomponent_free (valarm);
	icalcomponent_free (vevent);
	icalcomponent_free (vcalendar);
	// icaltimezone_free() below will free the component if it uses it
	if (!icaltz || vtimezone != icaltimezone_get_component (icaltz))
		icalcomponent_free (vtimezone);
	g_free (tzid);
	if (icaltz)
		icaltimezone_free (icaltz, TRUE);

	return result;
}

/**
 * Convert icaltimetype to string in UTC format. Date-only values are set
 * to occur at 00:00:00 of the given day.
 *
 * @param   tt
 *      the value which needs to be converted, either defined in UTC or relative to icaltz
 * @param   icaltz
 *      the default time zone for the event (see eas_cal_info_translator_parse_request()
 *      comment about events with more than one time zone)
 * @return
 *      allocated string, caller must free it *using free()* (not g_free())
 */
static char *_ical2eas_convert_icaltime_to_utcstr(icaltimetype tt, const icaltimezone* icaltz)
{
	char* timestamp = NULL;

	// first tell libical what we know about the time zone
	if (icaltz && !icaltime_is_utc(tt))
		tt = icaltime_set_timezone (&tt, icaltz);

	// then make it a date-time value
	tt.is_date = 0;

	// finally convert to UTC
	tt = icaltime_convert_to_zone(tt, icaltimezone_get_utc_timezone());

	// don't depend on libical string buffer, allocate anew
	timestamp = icaltime_as_ical_string_r (tt);
	return timestamp;
}

/**
 * Process the RRULE (recurrence rule) property during parsing of an iCalendar VEVENT component
 *
 * @param  prop
 *      Pointer to the RRULE property
 * @param  appData
 *      Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_rrule (icalproperty* prop, xmlNodePtr appData, struct icaltimetype *startTime,
				     const icaltimezone* icaltz)
{
	// Get the iCal RRULE property
	struct icalrecurrencetype rrule = icalproperty_get_rrule (prop);

	// Create a new <Recurrence> element to contain the recurrence sub-elements
	xmlNodePtr recurNode = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_RECURRENCE, NULL);

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
	switch (rrule.freq) {
	case ICAL_SECONDLY_RECURRENCE:
	case ICAL_MINUTELY_RECURRENCE:
	case ICAL_HOURLY_RECURRENCE:
		g_warning ("DATA LOSS: cannot encode secondly/minutely/hourly recurrence in EAS.");
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
		g_warning ("RRULE with no recurrence type.");
		break;
	}
	// Note: don't add this to the XML yet: if we encounter an "nth day in the month" value
	// blow we need to change this

	//
	// COUNT & UNTIL
	//
	// Note: count and until are mutually exclusive in both formats, with count taking precedence
	if (rrule.count) {
		// EAS specifies a maximum value of 999 for the Occurrences element
		if (rrule.count > 999) {
			g_warning ("DATA LOSS: RRULE had recurrence count of %d, maximum is 999.", rrule.count);
			rrule.count = 999;
		}
		xmlValue = g_strdup_printf ("%d", rrule.count);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_OCCURRENCES, (const xmlChar*) xmlValue);
		g_free (xmlValue);
		xmlValue = NULL;
	} else if (!icaltime_is_null_time (rrule.until)) {
		/* Exchange seems to have exclusive end date, while iCalendar is inclusive. Add one second. */
		struct icaltimetype until = rrule.until;
		char *modified = _ical2eas_convert_icaltime_to_utcstr(until, icaltz);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_UNTIL, (const xmlChar*)modified);
		free (modified);
	}

	//
	// INTERVAL
	//
	if (rrule.interval) {
		// EAS specifies a maximum value of 999 for the Interval element
		if (rrule.interval > 999) {
			g_warning ("DATA LOSS: RRULE had recurrence interval of %d, maximum is 999.", rrule.interval);
			rrule.interval = 999;
		}
		// Only write the Interval element if it's greater than 1;
		// 1 is te default (i.e. every day)
		if (rrule.interval > 1) {
			xmlValue = g_strdup_printf ("%d", rrule.interval);
			xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_INTERVAL, (const xmlChar*) xmlValue);
			g_free (xmlValue);
			xmlValue = NULL;
		}
	}

	//
	// BYDAY
	//
	// icalrecurrencetype arrays are terminated with ICAL_RECURRENCE_ARRAY_MAX unless they're full
	for (index = 0;
	     (index < ICAL_BY_DAY_SIZE) && (rrule.by_day[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++) {
		enum icalrecurrencetype_weekday icalDayOfWeek = icalrecurrencetype_day_day_of_week (rrule.by_day[index]);
		gint icalDayPosition = icalrecurrencetype_day_position (rrule.by_day[index]);

		switch (icalDayOfWeek) {
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
			g_warning ("Found by-day RRULE with an empty day value");
			break;
		}

		// Now process the position part
		if (icalDayPosition != 0) {
			if (icalDayPosition < -1) {
				g_warning ("DATA LOSS: EAS cannot encode RRULE position value of %d", icalDayPosition);
				// For now, convert all large naegative values (meaning nth from the end of the month)
				// to 5 (meaning last of the month)
				icalDayPosition = 5;
			} else if (icalDayPosition == -1) {
				// Convert to the equivalent EAS value
				// (both mean "last instance in the month")
				icalDayPosition = 5;
			}

			// Check if we've already processed a position part from one of the other recurrence days
			// (EAS has no way of encoding different position values for different days)
			if (weekOfMonth && (weekOfMonth != icalDayPosition)) {
				g_warning ("DATA LOSS: Position %d already stored for this recurrence; ignoring value of %d", weekOfMonth, icalDayPosition);
			} else {
				weekOfMonth = icalDayPosition;
			}
		}
	}// end of for loop

	if (dayOfWeek) {
		//g_debug("RECURRENCE: DayOfWeek value = %d (0x%08X)", dayOfWeek, dayOfWeek);
		xmlValue = g_strdup_printf ("%d", dayOfWeek);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DAYOFWEEK, (const xmlChar*) xmlValue);
		g_free (xmlValue);
		xmlValue = NULL;
	}
	if (weekOfMonth) {
		// Set the Type value to 3 ("Recurs monthly on the nth day")
		recurType = 3;

		xmlValue = g_strdup_printf ("%d", weekOfMonth);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_WEEKOFMONTH, (const xmlChar*) xmlValue);
		g_free (xmlValue);
		xmlValue = NULL;
	}

	// And now we can add the Type element too
	xmlValue = g_strdup_printf ("%d", recurType);
	xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_TYPE, (const xmlChar*) xmlValue);
	g_free (xmlValue);
	xmlValue = NULL;

	//
	// WKST
	//
	/*
	if (rrule.week_start)
	{
		// EAS value is 0=Sunday..6=Saturday
		// libical value is 0=NoDay, 1=Sunday..7=Saturday
		xmlValue = g_strdup_printf("%d", rrule.week_start - 1);
		xmlNewTextChild(recurNode, NULL, (const xmlChar*)EAS_NAMESPACE_CALENDAR EAS_ELEMENT_FIRSTDAYOFWEEK, (const xmlChar*)xmlValue);
		g_free(xmlValue); xmlValue = NULL;
	} */

	//
	// BYMONTH
	//
	for (index = 0;
	     (index < ICAL_BY_MONTH_SIZE) && (rrule.by_month[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++) {
		if (monthOfYear == 0) {
			monthOfYear = rrule.by_month[index];
		} else {
			// We've already set monthOfyear: EAS only supports a single monthly recurrence
			// (unlike days where we can repeat on many days of the week)
			g_warning ("DATA LOSS: Already set to recur on month %d, discarding recurrence info for month %d", monthOfYear, rrule.by_month[index]);
		}
	}

	if (monthOfYear <= 0 && (recurType == 5 || recurType == 6)) {
		//if we have yearly recurrence, it is mandatory to have a month item - get it from startTime
		monthOfYear = startTime->month;
	}

	if (monthOfYear > 0) {
		xmlValue = g_strdup_printf ("%d", monthOfYear);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_MONTHOFYEAR, (const xmlChar*) xmlValue);
		g_free (xmlValue);
		xmlValue = NULL;
	}

	//
	// BYMONTHDAY
	//
	for (index = 0;
	     (index < ICAL_BY_MONTHDAY_SIZE) && (rrule.by_month_day[index] != ICAL_RECURRENCE_ARRAY_MAX);
	     index++) {
		if (dayOfMonth == 0) {
			dayOfMonth = rrule.by_month_day[index];
		} else {
			// EAS only supports a single occurrence of DayOfMonth
			g_warning ("DATA LOSS: Already set to recur on day %d of the month, discarding recurrence info for day %d of month", dayOfMonth, rrule.by_month_day[index]);
		}
	}
	if (dayOfMonth <= 0 && (recurType == 2 || recurType == 5)) {
		//if we have monthly or yearly recurrence, but have not yet set the day, we need to do this
		//get it from start date
		dayOfMonth = startTime->day;
	}



	if (dayOfMonth > 0) {
		xmlValue = g_strdup_printf ("%d", dayOfMonth);
		xmlNewTextChild (recurNode, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DAYOFMONTH, (const xmlChar*) xmlValue);
		g_free (xmlValue);
		xmlValue = NULL;
	}


}

static void set_xml_body_text (xmlNodePtr appData, const char *text)
{
	xmlNodePtr bodyNode = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY, NULL);
	xmlNewTextChild (bodyNode, NULL, (const xmlChar*) EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY_TYPE, (const xmlChar*) EAS_BODY_TYPE_PLAINTEXT);
	xmlNewTextChild (bodyNode, NULL, (const xmlChar*) EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_TRUNCATED, (const xmlChar*) EAS_BOOLEAN_FALSE);
	xmlNewTextChild (bodyNode, NULL, (const xmlChar*) EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_DATA, (const xmlChar*) text);
	// All other fields are optional
}


/**
 * Ensure that all XML properties are set. Otherwise removing
 * properties is not possible. It would be nice if this could be
 * limited to properties which exist on the server, but the daemon
 * doesn't track that information.
 */
static void
set_missing_calendar_properties (xmlNodePtr node, gboolean exception)
{
	/*
	 * The default values must match the iCalendar 2.0 defaults,
	 * so that a missing iCalendar 2.0 property leads to the right
	 * explicit value in XML.
	 */
	static const struct {
		const char *name;
		const char *def;
		gboolean notForExceptions;
	} elements[] = {
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SUBJECT, "" },
		/* no useful defaults for start and end time */
		/* EAS_NAMESPACE_CALENDAR EAS_ELEMENT_STARTTIME, */
		/* EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ENDTIME, */
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_LOCATION, "" },
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, EAS_SENSITIVITY_NORMAL },
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_BUSYSTATUS, EAS_BUSYSTATUS_BUSY },

		/* can only be set on parent event */
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ALLDAYEVENT, "0", TRUE },

		/*
		 * organizer information is added back by Exchange
		 * anyway (by setting the calendar owner), so don't
		 * bother sending empty properties
		 */
		/* EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_EMAIL, */
		/* EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_NAME, */

		/* adding <Reminder> doesn't seem to be necessary to remove a reminder */
		/* EAS_NAMESPACE_CALENDAR EAS_ELEMENT_REMINDER */

		/* Can be set like a text element, but not on
		   exceptions. <Categories> cannot be added without
		   entries to an exception (which is allowed for the
		   parent). Has the effect that exceptions cannot
		   remove the categories of their parent. Attendees
		   also cannot differ from the parent.
		*/
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_CATEGORIES, "", TRUE },
		{ EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ATTENDEES, "", TRUE },

		/*
		 * removing a recurrence rule is more difficult, not
		 * currently supported: a <Type> has to be set, but there
		 * is no value for "does not recur"
		 */
		/* { EAS_NAMESPACE_CALENDAR EAS_ELEMENT_RECURRENCE, "" }, */

		/*
		 * An empty <Exceptions> element is okay, but doesn't
		 * help for removing EXDATE or detached recurrence: we
		 * would have to send an explicit <Exception> with
		 * <Exception_Deleted>0 and the right
		 * <Exception_StartTime> to reset an exception on the
		 * server. The daemon doesn't know about stored
		 * exceptions and the iCalendar 2.0 item doesn't tell
		 * us, so we can't do that here.
		 *
		 * TODO BMC #24290: The solution might be to add special
		 * X-ACTIVESYNCD-OLD-EXDATE values when sending data to
		 * the local client. Assuming that we get them back, we
		 * could then add the right <Exception> elements.
		 */
		/* { EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS, "" }, */
		{ NULL, NULL }
	};
	int i;

	for (i = 0; elements[i].name; i++)
		if ((!exception || !elements[i].notForExceptions) &&
		    !is_element_set (node, elements[i].name))
			xmlNewTextChild (node, NULL, (const xmlChar *)elements[i].name, (const xmlChar *)elements[i].def);

	/* special case for body */
	if (!is_element_set (node, EAS_NAMESPACE_AIRSYNCBASE EAS_ELEMENT_BODY))
		set_xml_body_text (node, "");
}

/**
 * Process the VEVENT component during parsing of an iCalendar
 *
 * @param  vevent
 *      Pointer to the iCalendar VEVENT component
 * @param  appData
 *      Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_vevent (icalcomponent* vevent, xmlNodePtr appData, icaltimezone* icaltz,   gboolean exception)
{
	if (vevent) {
		xmlNodePtr categories = NULL;
		xmlNodePtr exceptions = NULL;
		xmlNodePtr attendees = NULL;
		//xmlNodePtr icalExtns = NULL;
		struct icaltimetype startTime, endTime;
		icalproperty* prop;

		// Should be set when iterating over properties, but better don't
		// rely on it and initialize upfront before checking for "all day"
		// property after the loop. Keeps static code analysis happy.
		memset (&startTime, 0, sizeof (startTime));
		memset (&endTime, 0, sizeof (endTime));

		for (prop = icalcomponent_get_first_property (vevent, ICAL_ANY_PROPERTY);
		     prop;
		     prop = icalcomponent_get_next_property (vevent, ICAL_ANY_PROPERTY)) {
			const icalproperty_kind prop_type = icalproperty_isa (prop);
			switch (prop_type) {
				// SUMMARY
			case ICAL_SUMMARY_PROPERTY:
				xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SUBJECT, (const xmlChar*) icalproperty_get_summary (prop));
				break;

				// DTSTAMP
			case ICAL_DTSTAMP_PROPERTY: {
				gchar* modified = NULL;
				const gchar* timestamp = icalproperty_get_value_as_string (prop);
				if (!g_str_has_suffix (timestamp, "Z")) {
					modified = g_strconcat (timestamp, "Z", NULL);
				} else {
					modified = g_strdup (timestamp);
				}
				if (!exception)
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DTSTAMP, (const xmlChar*) modified);
				g_debug ("dtstamp cleanup");
				g_free (modified);
			}
			break;

			// DTSTART
			case ICAL_DTSTART_PROPERTY: {
				struct icaltimetype tt;
				char* modified = NULL;

				//get start time, convert it to UTC and suffix Z onto it
				tt = icalproperty_get_dtstart (prop);
				modified = _ical2eas_convert_icaltime_to_utcstr(tt, icaltz);
				xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_STARTTIME, (const xmlChar*) modified);
				// And additionally store the start time so we can calculate the AllDayEvent value later
				startTime = tt;
				g_debug ("dtstart cleanup");
				free (modified);
			}
			break;

			// DTEND
			case ICAL_DTEND_PROPERTY: {
				struct icaltimetype tt;
				char* modified = NULL;

				//get end time, convert it to UTC and suffix Z onto it
				tt = icalproperty_get_dtend (prop);
				modified = _ical2eas_convert_icaltime_to_utcstr(tt, icaltz);
				xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ENDTIME, (const xmlChar*) modified);
				// And additionally store the end time so we can calculate the AllDayEvent value later
				endTime = icalproperty_get_dtend (prop);
				free (modified);
			}
			break;

			// LOCATION
			case ICAL_LOCATION_PROPERTY:
				xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_LOCATION, (const xmlChar*) icalproperty_get_value_as_string (prop));
				break;

				// UID
			case ICAL_UID_PROPERTY:
				if (!exception)
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_UID, (const xmlChar*) icalproperty_get_value_as_string (prop));
				break;

				// CLASS
			case ICAL_CLASS_PROPERTY: {
				icalproperty_class classValue = icalproperty_get_class (prop);
				switch (classValue) {
				case ICAL_CLASS_CONFIDENTIAL:
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*) EAS_SENSITIVITY_CONFIDENTIAL);
					break;
				case ICAL_CLASS_PRIVATE:
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*) EAS_SENSITIVITY_PRIVATE);
					break;
				default: // PUBLIC or NONE (iCalendar doesn't distinguish between 0 (Normal) and 1 (Personal))
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_SENSITIVITY, (const xmlChar*) EAS_SENSITIVITY_NORMAL);
					break;
				}
			}
			break;

			// TRANSP
			case ICAL_TRANSP_PROPERTY: {
				icalproperty_transp transp = icalproperty_get_transp (prop);
				if (transp == ICAL_TRANSP_TRANSPARENT) {
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_BUSYSTATUS, (const xmlChar*) EAS_BUSYSTATUS_FREE);
				} else { // OPAQUE
					// iCalendar doesn't distinguish between 1 (Tentative), 2 (Busy), 3 (Out of Office)
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_BUSYSTATUS, (const xmlChar*) EAS_BUSYSTATUS_BUSY);
				}
			}
			break;

			// CATEGORIES
			case ICAL_CATEGORIES_PROPERTY: {
				if (categories == NULL) {
					categories = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_CATEGORIES, NULL);
				}
				xmlNewTextChild (categories, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_CATEGORY, (const xmlChar*) icalproperty_get_categories (prop));
			}
			break;

			// ORGANIZER
			case ICAL_ORGANIZER_PROPERTY: {
				icalparameter* cnParam = NULL;

				// Get the e-mail address
				if (!exception)
					xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_EMAIL, (const xmlChar*) icalproperty_get_organizer (prop));

				// Now check for a name in the (optional) CN parameter
				cnParam = icalproperty_get_first_parameter (prop, ICAL_CN_PARAMETER);
				if (cnParam) {
					if (!exception)
						xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ORGANIZER_NAME, (const xmlChar*) icalparameter_get_cn (cnParam));
				}
			}
			break;
			// ATTENDEES
			case ICAL_ATTENDEE_PROPERTY: {
				icalparameter* param = NULL;
				xmlNodePtr attendee = NULL;
				if (attendees == NULL) {
					attendees = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ATTENDEES, NULL);
				}
				attendee = xmlNewChild (attendees, NULL, (const xmlChar*) EAS_ELEMENT_ATTENDEE, NULL);

				// Get the e-mail address
				xmlNewTextChild (attendee, NULL, (const xmlChar*) EAS_ELEMENT_ATTENDEE_EMAIL, (const xmlChar*) icalproperty_get_value_as_string (prop));

				// Now check for a name in the  CN parameter
				// Name is a required element for Activesync, so if it is not present, then use the email
				param = icalproperty_get_first_parameter (prop, ICAL_CN_PARAMETER);
				if (param) {
					xmlNewTextChild (attendee, NULL, (const xmlChar*) EAS_ELEMENT_ATTENDEE_NAME, (const xmlChar*) icalparameter_get_cn (param));
				} else {
					xmlNewTextChild (attendee, NULL, (const xmlChar*) EAS_ELEMENT_ATTENDEE_NAME, (const xmlChar*) icalproperty_get_value_as_string (prop));
				}
			}
			break;
			// RRULE
			case ICAL_RRULE_PROPERTY: {
				_ical2eas_process_rrule (prop, appData, &startTime, icaltz);
			}
			break;

			// RDATE
			case ICAL_RDATE_PROPERTY: {

				const gchar* start = NULL;
				gchar *modified = NULL;

				xmlNodePtr exception = NULL;

				// Create the <Exceptions> container element if not already present
				if (exceptions == NULL) {
					exceptions = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS, NULL);
				}

				// Now create the <Exception> element
				exception = xmlNewChild (exceptions, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTION, NULL);

				start = icalproperty_get_value_as_string (prop);
				if (strlen (start) <= 9) {
					if (!g_str_has_suffix (start, "Z")) {
						modified = g_strconcat (start, "T000000Z", NULL);
					} else {
						//need to add midnight timestamp before last characters
						//first remove last character
						gchar * temp = g_strndup (start, (strlen (start) - 1));
						//then concatenate timestamp + "Z"
						modified = g_strconcat (temp, "T000000", "Z", NULL);
						g_free (temp);
					}
				} else {
					if (!g_str_has_suffix (start, "Z")) {
						modified = g_strconcat (start, "Z", NULL);
					} else {
						modified = g_strdup (start);
					}
				}
				xmlNewTextChild (exception, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONSTARTTIME, (const xmlChar*) modified);
				g_free (modified);
			}
			break;

			// EXDATE
			case ICAL_EXDATE_PROPERTY: {
				// EXDATE consists of a list of date/times, comma separated.
				// However, libical breaks this up for us and converts it into
				// a number of single-value properties.
				struct icaltimetype tt;
				char *modified = NULL;

				xmlNodePtr exception = NULL;

				// Create the <Exceptions> container element if not already present
				if (exceptions == NULL) {
					exceptions = xmlNewChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS, NULL);
				}

				// Now create the <Exception> element
				exception = xmlNewChild (exceptions, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTION, NULL);
				xmlNewTextChild (exception, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_DELETED, (const xmlChar*) EAS_BOOLEAN_TRUE);

				tt = icalproperty_get_exdate (prop);
				// TODO: handle VALUE=DATE for events which are not all-day events
				modified = _ical2eas_convert_icaltime_to_utcstr (tt, icaltz);
				xmlNewTextChild (exception, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONSTARTTIME, (const xmlChar*) modified);
				free (modified);
			}
			break;

			// DESCRIPTION
			case ICAL_DESCRIPTION_PROPERTY: {

				// See [MS-ASAIRS] for format of the <Body> element:
				// http://msdn.microsoft.com/en-us/library/dd299454(v=EXCHG.80).aspx
				set_xml_body_text (appData, icalproperty_get_description(prop));
			}
			break;

			default: {
				// Any other unmapped properties get stored as extension elements under the
				// <activesyncd:iCalExtensions> collection element.
				/*
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
				*/
			}
			break;
			}// end of switch
		}// end of for loop




		// Add an <AllDayEvent> element if both the start and end dates have no times
		// (ie. just dates, with time set to midnight) and are 1 day apart
		// (from [MS-ASCAL]: "An item marked as an all day event is understood to begin
		// on midnight of the current day and to end on midnight of the next day.")
		// This description seems to limit AllDayEvent to events covering only one
		// day. In practice, the end time may also be at midnight of any of the following
		// days, to allow for multi-day all-day events.
		if (startTime.hour == 0 && startTime.minute == 0 && startTime.second == 0 && endTime.hour == 0 && endTime.minute == 0 && endTime.second == 0) {
			xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ALLDAYEVENT, (const xmlChar*) EAS_BOOLEAN_TRUE);
		} else {
			xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_ALLDAYEVENT, (const xmlChar*) EAS_BOOLEAN_FALSE);
		}


		// ensure that all properties are set so that missing ones
		// really get removed on the server
		set_missing_calendar_properties (appData, exception);
	}
}


/**
 * Process the VALARM component during parsing of an iCalendar
 *
 * @param  valarm
 *      Pointer to the iCalendar VALARM component
 * @param  appData
 *      Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_valarm (icalcomponent* valarm, xmlNodePtr appData)
{
	if (valarm) {
		// Just need to get the TRIGGER property
		icalproperty* prop = icalcomponent_get_first_property (valarm, ICAL_TRIGGER_PROPERTY);
		if (prop) {
			struct icaltriggertype trigger = icalproperty_get_trigger (prop);

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
			g_snprintf (minutes_buf, 6, "%d", minutes);

			xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_REMINDER, (const xmlChar*) minutes_buf);
		}
	}
}


/**
 * Parse the STANDARD and DAYLIGHT subcomponents of VTIMEZONE.
 * Using one function for both as their formats are identical.
 *
 * @param  subcomponent
 *      The STANDARD or DAYLIGHT subcomponent of the VTIMEZONE component
 * @param  timezone
 *      The EAS timezone structure to parse this subcomponent into
 * @param  type
 *      Determines whether subcomponent is a STANDARD or a DAYLIGHT
 */
static void _ical2eas_process_xstandard_xdaylight (icalcomponent* subcomponent, EasTimeZone* timezone, icalcomponent_kind type)
{
	if (subcomponent) {
		// Determine whether we've been passed a STANDARD or DAYLIGHT component
		// and get a pointer to the appropriate time struct
		const gboolean isStandardTime = (type == ICAL_XSTANDARD_COMPONENT);
		EasSystemTime* easTimeStruct = (isStandardTime ? (& (timezone->StandardDate)) : (& (timezone->DaylightDate)));

		// Get the properties we're interested in. Note RRULE is optional but the rest are mandatory
		icalproperty* dtStart = icalcomponent_get_first_property (subcomponent, ICAL_DTSTART_PROPERTY);
		icalproperty* rrule = icalcomponent_get_first_property (subcomponent, ICAL_RRULE_PROPERTY);
		icalproperty* tzOffsetTo = icalcomponent_get_first_property (subcomponent, ICAL_TZOFFSETTO_PROPERTY);
		icalproperty* tzOffsetFrom = icalcomponent_get_first_property (subcomponent, ICAL_TZOFFSETFROM_PROPERTY);

		// Get the values of the properties
		// Note: icalproperty_get_tzoffsetto() and icalproperty_get_tzofsetfrom() return offsets as seconds.
		const icaltimetype dtStartValue = icalproperty_get_dtstart (dtStart);
		const int tzOffsetToValueMins = icalproperty_get_tzoffsetto (tzOffsetTo) / SECONDS_PER_MINUTE;
		const int tzOffsetFromValueMins = icalproperty_get_tzoffsetfrom (tzOffsetFrom) / SECONDS_PER_MINUTE;

		struct icalrecurrencetype rruleValue;
		if (rrule) {
			rruleValue = icalproperty_get_rrule (rrule);
		} else {
			icalrecurrencetype_clear (&rruleValue);
		}

		if (isStandardTime) {
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
		} else { // It's daylight time
			// As with the bias above, this value is inverted from our usual understanding of it. e.g. If a
			// daylight saving phase adds 1 hour to the standard phase, the DaylightBias value is -60. Note
			// that DaylightBias and StandardBias are the additional offsets *relative to the base Bias value*
			// (rather than absolute offsets from UTC). We can calculate the daylight bias easily as follows:
			timezone->DaylightBias = tzOffsetFromValueMins - tzOffsetToValueMins;
		}

		// Handle recurrence information if present
		if (rrule) {
			// We can assume FREQ=YEARLY: EAS only supports annually recurring timezone changes
			short byMonth = rruleValue.by_month[0];
			short byDayRaw = rruleValue.by_day[0];
			icalrecurrencetype_weekday byDayWeekday = icalrecurrencetype_day_day_of_week (byDayRaw);
			/** 0 == any of day of week. 1 == first, 2 = second, -2 == second to last, etc */
			int byDayPosition = icalrecurrencetype_day_position (byDayRaw);

			easTimeStruct->Year = 0; // Always 0 if we have recurrence

			//AG - We can't assume that the month field is in the RRULE, so check if the month is valid
			// and if not, then get it from the dtStartValue
			if (1 <= byMonth && byMonth <= 12) {
				easTimeStruct->Month = byMonth;
			} else {
				easTimeStruct->Month = dtStartValue.month;
			}

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
			g_assert ( (-5 <= byDayPosition && byDayPosition <= -1) || (1 <= byDayPosition && byDayPosition <= 5));
			if (byDayPosition > 0) {
				easTimeStruct->Day = byDayPosition;
			} else { // byDayPosition is negative
				// Convert -1 to 5, -2 to 4, etc. (see above for reason why)
				easTimeStruct->Day = 6 + byDayPosition;
			}

			// Don't want to rely on enum values in icalrecurrencetype_weekday so use a switch statement to set the DayOfWeek
			switch (byDayWeekday) {
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
		} else { // No recurrence information: just a one-off time change, so we set an absolute date value for EAS
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
		icalproperty_free (dtStart);
		icalproperty_free (tzOffsetFrom);
		icalproperty_free (tzOffsetTo);
		if (rrule) {
			icalproperty_free (rrule);
		}
	}
}

/**
 * Process the VTIMEZONE component during parsing of an iCalendar
 *
 * @param  vtimezone
 *      Pointer to the iCalendar VTIMEZONE component
 * @param  forceTimezone
 *      Send UTC if vtimezone is empty; useful
 *      in one particular case (recurring all-day property with
 *      exceptions), otherwise Exchange didn't handle the event
 *      properly (dropped exceptions, see BMC #22780), and
 *      in another where not sending any time zone information
 *      caused some default to be added (123together.com)
 * @param  appData
 *      Pointer to the <ApplicationData> element to add parsed elements to
 */
static void _ical2eas_process_vtimezone (icalcomponent* vtimezone, gboolean forceTimezone, xmlNodePtr appData)
{
	if (vtimezone) {
		EasTimeZone timezoneStruct;
		gchar* timezoneBase64 = NULL;
		icalcomponent* subcomponent = NULL;
		icalproperty* tzid = icalcomponent_get_first_property (vtimezone, ICAL_TZID_PROPERTY);

		// all empty == UTC as default
		memset (&timezoneStruct, 0, sizeof (EasTimeZone));

		// Only one property in a VTIMEZONE: the TZID
		if (tzid) {
			// Get the ASCII value from the iCal
			const char* tzidValue8 = icalproperty_get_value_as_string (tzid);

			// Convert to Unicode, max. 32 chars (including the trailing 0)
			glong words;
			gunichar2* tzidValue16 = g_utf8_to_utf16 ((const gchar *)tzidValue8, 31, NULL, &words, NULL);

			// Copy this into the EasTimeZone struct as both StandardName and DaylightName
			if (tzidValue16) {
				memcpy (& (timezoneStruct.StandardName), tzidValue16,
					MIN(sizeof (gunichar2) * words, sizeof(timezoneStruct.StandardName)));
				memcpy (& (timezoneStruct.DaylightName), tzidValue16,
					MIN(sizeof (gunichar2) * words, sizeof(timezoneStruct.DaylightName)));
				g_free (tzidValue16);
			}
		}

		// Now process the STANDARD and DAYLIGHT subcomponents
		for (subcomponent = icalcomponent_get_first_component (vtimezone, ICAL_ANY_COMPONENT);
		     subcomponent;
		     subcomponent = icalcomponent_get_next_component (vtimezone, ICAL_ANY_COMPONENT)) {
			_ical2eas_process_xstandard_xdaylight (subcomponent, &timezoneStruct, icalcomponent_isa (subcomponent));
		}

		// Write the timezone into the XML, base64-encoded.
		timezoneBase64 = g_base64_encode ( (const guchar *) (&timezoneStruct), sizeof (EasTimeZone));
		xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_TIMEZONE, (const xmlChar*) timezoneBase64);
		g_free (timezoneBase64);
	} else if (forceTimezone) {
		// This is Exchange 2010's idea of UTC. Corresponds to:
		// bias 0, standard bias 0, daylight bias 0,
		// standard '(UTC) Coordinated Universal Time',
		// daylight '(UTC) Coordinated Universal Time'
		xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_TIMEZONE, (const xmlChar*)"AAAAACgAVQBUAEMAKQAgAEMAbwBvAHIAZABpAG4AYQB0AGUAZAAgAFUAbgBpAHYAZQByAHMAYQBsACAAVABpAG0AZQAAAAAAAAAAAAAAAAAAAAAAAAAAACgAVQBUAEMAKQAgAEMAbwBvAHIAZABpAG4AYQB0AGUAZAAgAFUAbgBpAHYAZQByAHMAYQBsACAAVABpAG0AZQAAAAAAAAAAAAAAAAAAAAAAAAAAAA==");
	}
}

/** compare icaltimetype in qsort() */
static int comparetimes(const void *a, const void *b)
{
	return icaltime_compare (*(const icaltimetype *)a,
				 *(const icaltimetype *)b);
}

/**
 * Check whether a recurrence rule covers all detached recurrences.
 * Parameters see calculateRecurrence().
 *
 * Done by expanding the rrule with the first detached recurrence
 * as start time and then checking against the next recurrence
 * until one is found which is not covered or we are done.
 */
static gboolean
rruleMatches (const struct icalrecurrencetype *recur,
	      int numDetached, const icaltimetype *detached)
{
	icalrecur_iterator* ritr = icalrecur_iterator_new (*recur, detached[0]);
	struct icaltimetype instance;
	int next = 0;
	gboolean res = TRUE;
	while (res &&
	       next < numDetached &&
	       (instance = icalrecur_iterator_next (ritr),
		!icaltime_is_null_time (instance))) {
		int cmp = icaltime_compare (detached[next], instance);
		if (cmp == 0) {
			/* okay, move on to next detached recurrence */
			next++;
		} else if (cmp < 0) {
			/* failure, the current recurrence was not covered */
			res = FALSE;
		}
	}

	if (res &&
	    (next < numDetached /* iterator did not return enough results */ ||
	     (instance = icalrecur_iterator_next (ritr),
	      !icaltime_is_null_time (instance)) /* iterator returns too many results */))
		res = FALSE;

	icalrecur_iterator_free (ritr);
	return res;
}

/**
 * Generate recurrence rule covering the detached recurrence times.
 * The time zone of the first detached recurrence is used.
 *
 * @param   numDetached
 *      Number of entries in detached array, > 0.
 * @param   detached
 *      date-times of detached recurrences, with time zone,
 *      sorted from oldest to most recent
 */
static struct icalrecurrencetype
calculateRecurrence (int numDetached, const icaltimetype *detached)
{
	static const struct icalrecurrencetype recur = {
		.freq = ICAL_NO_RECURRENCE,
		.interval = 1,
		.week_start = ICAL_MONDAY_WEEKDAY,
		.by_second = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_minute = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_hour = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_day = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_month_day = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_year_day = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_week_no = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_month = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX },
		.by_set_pos = { ICAL_RECURRENCE_ARRAY_MAX, ICAL_RECURRENCE_ARRAY_MAX }
	};
	int freq;
	struct icaldurationtype delta;
	if (numDetached >= 2)
		delta = icaltime_subtract (detached[1], detached[0]);
	else
		// keep compiler happy (doesn't detect that delta is not used in this case)
		memset (&delta, 0, sizeof(delta));

	// try yearly, monthly, ... , secondly recurrence
	for (freq = ICAL_YEARLY_RECURRENCE;
	     freq >= ICAL_SECONDLY_RECURRENCE;
	     freq--) {
		struct icalrecurrencetype res = recur;
		res.freq = freq;
		res.until = detached[numDetached - 1];
			/* icaltime_convert_to_zone (detached[numDetached - 1],
			   icaltimezone_get_utc_timezone ()); */

		switch (freq) {
		case ICAL_YEARLY_RECURRENCE:
			res.by_month[0] = detached[0].month;
			res.by_month_day[0] = detached[0].day;
			break;
		case ICAL_MONTHLY_RECURRENCE:
			res.by_month_day[0] = detached[0].day;
			break;
		case ICAL_WEEKLY_RECURRENCE:
			res.by_day[0] = icaltime_day_of_week(detached[0]);
			break;
		}

		// Try special case first:
		// calculate delta between first and second recurrence,
		// use that to set an interval. If we only have two
		// recurrences, then this will result in an rrule with
		// no need for EXDATEs. If we have more, perhaps we are
		// lucky and the rule still matches the rest.
		if (numDetached >= 2) {
			res.interval = 0;
			switch (freq) {
				/*
				 * interval > 1 replaced with interval = 1 by Exchange, avoid generating such rules
				 * case ICAL_YEARLY_RECURRENCE:
				 * res.interval = detached[1].year - detached[0].year;
				 * break;
				 */
			case ICAL_MONTHLY_RECURRENCE: {
				int months = detached[1].month - detached[0].month;
				if (months <= 0) {
					months += 12;
				}
				res.interval = months;
				break;
			}
			case ICAL_DAILY_RECURRENCE:
				res.interval = delta.weeks * 7 + delta.days;
				break;
			case ICAL_HOURLY_RECURRENCE:
				res.interval = (delta.weeks * 7 + delta.days) * 24 + delta.hours;
				break;
			case ICAL_MINUTELY_RECURRENCE:
				res.interval = ((delta.weeks * 7 + delta.days) * 24 + delta.hours) * 60 + delta.minutes;
				break;
			case ICAL_SECONDLY_RECURRENCE:
				res.interval = (((delta.weeks * 7 + delta.days) * 24 + delta.hours) * 60 + delta.minutes) * 60 + delta.seconds;
				break;
			}
			if (res.interval) {
				/*
				 * start with interval + 1, because if a time zone change
				 * happens between the two detached recurrences, the actual
				 * delta in time will have +- 1 hour
				 */
				for (res.interval++;
				     res.interval > 1;
				     res.interval--) {
					if (rruleMatches (&res, numDetached, detached))
						return res;
				}
			}
		}

		// general case
		res.interval = 1;
		if (rruleMatches (&res, numDetached, detached))
			return res;
	}

	// nothing found, will probably fail later somewhere
	return recur;
}

/**
 * Iterate over all regular recurrences of an artificial parent
 * event and add EXDATEs for all recurrences which do not
 * have a detached recurrence.
 */
static void calculateExdates (icalcomponent *parent,
			      const struct icalrecurrencetype *recur,
			      int numDetached,
			      icaltimetype *detached)
{
	icalrecur_iterator* ritr = icalrecur_iterator_new (*recur, detached[0]);
	struct icaltimetype instance;
	int next = 0;
	icalproperty *dtstart = icalcomponent_get_first_property (parent, ICAL_DTSTART_PROPERTY);
	icalparameter *tzid = icalproperty_get_first_parameter (dtstart, ICAL_TZID_PROPERTY);
	while ((instance = icalrecur_iterator_next (ritr),
		!icaltime_is_null_time (instance))) {
		int cmp = icaltime_compare (detached[next], instance);
		if (cmp == 0) {
			/* has detached recurrence, move on to next one */
			next++;
		} else if (cmp < 0) {
			/* Not matched?! Shouldn't happen, but so be it. */
			next++;
		} else {
			icalproperty *prop = icalproperty_vanew_exdate (instance,
									tzid ? icalparameter_new_clone (tzid) : (void *)0,
									(void *)0);
			icalcomponent_add_property (parent, prop);
		}
	}
	icalrecur_iterator_free (ritr);
}

/**
 * Parse an EasCalInfo structure and convert to EAS XML format
 *
 * @param  doc
 *      REDUNDANT PARAMETER: only required for debug output. TODO: remove this
 * @param  appData
 *      The top-level <ApplicationData> XML element in which to store all the parsed elements
 * @param  calInfo
 *      The EasCalInfo struct containing the iCalendar string to parse (plus a server ID)
 */
gboolean eas_cal_info_translator_parse_request (xmlDocPtr doc, xmlNodePtr appData, EasItemInfo* calInfo)
{
	gboolean success = FALSE;
	icalcomponent* ical = NULL;
	icaltimezone* icaltz = NULL;
	icaltimetype *recurrenceTimes = NULL;

	g_debug (" Cal Data: %s", calInfo->data);

	if (doc &&
	    appData &&
	    calInfo &&
	    (appData->type == XML_ELEMENT_NODE) &&
	    (g_strcmp0 ( (char*) (appData->name), EAS_ELEMENT_APPLICATIONDATA) == 0) &&
	    (ical = icalparser_parse_string (calInfo->data)) &&
	    (icalcomponent_isa (ical) == ICAL_VCALENDAR_COMPONENT)) {
		icalcomponent* vevent = NULL;
		icalcomponent* exceptionvevent = NULL;
		icalcomponent* vtimezone = NULL;
		icalproperty* tzid = NULL;
		struct icaltimetype tt;
		icalproperty* prop = NULL;
		gboolean forceTimezone = FALSE;
		xmlNodePtr exceptions = NULL;
		xmlNodePtr subNode   = NULL;
		icalcomponent* comp;

		// The code here assumes that there is either no or one VTIMEZONE,
		// and that if there is one, all time stamps use that zone. This is a slight
		// oversimplification (iCalendar 2.0 allows multiple different VTIMEZONEs
		// to be used, and both the Evolution and Google Calendar UI support that).
		//
		// TODO: choose one time zone for ActiveSync, properly convert times with other
		// time zones into it.

		vtimezone = icalcomponent_get_first_component (ical, ICAL_VTIMEZONE_COMPONENT);
		if (vtimezone)
			tzid = icalcomponent_get_first_property (vtimezone, ICAL_TZID_PROPERTY);

		if (tzid) {
			icaltz = icaltimezone_new();
			icaltimezone_set_component(icaltz, vtimezone);
		}

		// Use UTC time zone as fallback. May happen for all-day events (which need
		// no time zone) or events which truly were defined as local time. In the
		// latter case it would be better to use the system time zone, but we don't
		// know what that is. Besides, such events are broken by design and shouldn't occur.
		if (!icaltz)
			icaltz = icaltimezone_get_utc_timezone();
		if (!icaltz)
			goto error;
		vtimezone = icaltimezone_get_component (icaltz);
		tzid = vtimezone ?
			icalcomponent_get_first_property (vtimezone, ICAL_TZID_PROPERTY) :
			NULL;

		// Process the components of the VCALENDAR.
		// Don't make assumptions about any particular order, check RECURRENCE-ID
		// to find exceptions.
		for (comp = icalcomponent_get_first_component (ical, ICAL_VEVENT_COMPONENT);
		     comp;
		     comp = icalcomponent_get_next_component (ical, ICAL_VEVENT_COMPONENT)) {
			if (!icalcomponent_get_first_property (comp, ICAL_RECURRENCEID_PROPERTY)) {
				vevent = comp;
				break;
			}
		}

		/* regenerate artificial parent */
		if (vevent &&
		    !g_strcmp0 (icalcomponent_get_summary (vevent), ACTIVESYNCD_PSEUDO_EVENT)) {
			icalcomponent_remove_component (ical, vevent);
			icalcomponent_free (vevent);
			vevent = NULL;
		}

		if (!vevent) {
			// Create fake parent with same time zone and same UID
			// as first detached recurrence. It must have a recurrence
			// pattern which covers all detached recurrences (because
			// Exchange rejects those otherwise). In addition, it must
			// never be visible at any of the recurrences. Add EXDATE
			// for those recurrences not covered by detached recurrences.

			// Calculate "minimal" recurrence rule (= produces as little
			// recurrences as possible) based on RECURRENCE-IDs of the
			// the detached recurrences. The relevant time zone for
			// recurrences is the one of the fictious parent event.
			int numDetached = 1;
			struct icalrecurrencetype recur;
			int i;
			icalproperty *rid;
			icalparameter *tzid;

			vevent = icalcomponent_get_first_component (ical, ICAL_VEVENT_COMPONENT);
			if (!vevent)
				goto error;
			while (icalcomponent_get_next_component (ical, ICAL_VEVENT_COMPONENT))
				numDetached++;
			recurrenceTimes = g_malloc (sizeof(icaltimetype) * numDetached);
			for (vevent = icalcomponent_get_first_component (ical, ICAL_VEVENT_COMPONENT), i = 0;
			     vevent;
			     vevent = icalcomponent_get_next_component (ical, ICAL_VEVENT_COMPONENT), i++)
				recurrenceTimes[i] = _eas2cal_component_get_recurrenceid (vevent);
			qsort (recurrenceTimes, numDetached, sizeof(icaltimetype),
			       comparetimes);

			recur = calculateRecurrence (numDetached, recurrenceTimes);

			/* take tzid from any of the detached recurrences, but the
			   start time must come the oldest one */
			vevent = icalcomponent_get_first_component (ical, ICAL_VEVENT_COMPONENT);
			rid = icalcomponent_get_first_property (vevent, ICAL_RECURRENCEID_PROPERTY);
			tzid = icalproperty_get_first_parameter (rid, ICAL_TZID_PARAMETER);

			vevent =
				icalcomponent_vanew (ICAL_VEVENT_COMPONENT,
						     icalproperty_vanew_dtstart (recurrenceTimes[0],
										 tzid ? icalparameter_new_clone (tzid) : (void *)0,
										 (void *)0),
						     icalproperty_vanew_dtend (recurrenceTimes[0],
									       tzid ? icalparameter_new_clone (tzid) : (void *)0,
									       (void *)0),
						     icalproperty_new_uid (icalcomponent_get_uid (vevent)),
						     icalproperty_new_summary (ACTIVESYNCD_PSEUDO_EVENT),
						     icalproperty_new_transp (ICAL_TRANSP_TRANSPARENT),
						     icalproperty_new_rrule (recur),
						     (void *)0
						     );
			if (!vevent)
				goto error;

			// add EXDATE properties
			calculateExdates (vevent, &recur, numDetached, recurrenceTimes);

			icalcomponent_add_component (ical, vevent);
		}
		prop = icalcomponent_get_first_property (vevent, ICAL_DTSTART_PROPERTY);
		if (!prop)
			goto error;
		tt = icalproperty_get_dtstart (prop);

		// Recurring all-day events with detached recurrences got mangled
		// by Exchange 2010 (stored without error, but came back without
		// the exceptions). Sending a dummy UTC time zone definition avoided
		// the problem (see BMC #22780). We ignore the time zone when receiving all-day
		// events, so it is safe to send it (conversion back will produce
		// "nice" all-day event without time zone again, other peers should
		// deal with it okay, too - verified with Exchange/OWA).
		//
		// Also explicitly send a dummy UTC timezone if we use UTC.
		// Some Exchange 2010 servers were fine without it (time stamps
		// are in UTC anyway), but the installation on 123together.com
		// added its own local time when no explicit timezone was sent.
		if (tt.is_date || icaltz == icaltimezone_get_utc_timezone ())
			forceTimezone = TRUE;
		_ical2eas_process_vtimezone (vtimezone, forceTimezone, appData);
		_ical2eas_process_vevent (vevent, appData, icaltz, FALSE);

		// Always include <Exceptions> even if empty. Might have helped with removing
		// existing exceptions in an update (SyncEvolution testLinkedItemsRemoveNormal)
		// but didn't (BMC #22849).
		//
		// check if <Exceptions> node Already exists otherwise create a new <Exceptions> Node
		// TODO: why should it exist? We haven't created one yet, have we?
		for (subNode = appData->children; subNode; subNode = subNode->next) {
			if (subNode->type == XML_ELEMENT_NODE && g_strcmp0 ( (gchar*) subNode->name, EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS) == 0) {
				exceptions = subNode;
				break;
			}
		}
		if (!exceptions)
			exceptions = xmlNewTextChild (appData, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONS, NULL);


		for (exceptionvevent = icalcomponent_get_first_component (ical, ICAL_VEVENT_COMPONENT);
		     exceptionvevent;
		     exceptionvevent = icalcomponent_get_next_component (ical, ICAL_VEVENT_COMPONENT)) {
			xmlNodePtr exception;

			// ignore parent
			if (!icalcomponent_get_first_property (exceptionvevent, ICAL_RECURRENCEID_PROPERTY))
				continue;

			exception = xmlNewTextChild (exceptions, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTION, NULL);
			g_debug ("Processing multiple vevents as exceptions");

			_ical2eas_process_vevent (exceptionvevent, exception, icaltz, TRUE);
			_ical2eas_process_valarm (icalcomponent_get_first_component (exceptionvevent, ICAL_VALARM_COMPONENT), exception);

			//get recurrenceID ( which is a timestamp), convert it to UTC and suffix Z onto it
			prop = icalcomponent_get_first_property (exceptionvevent, ICAL_RECURRENCEID_PROPERTY);
			if(prop){
				char* modified = NULL;
				tt = icaltime_from_string(icalproperty_get_value_as_string (prop));
				modified = _ical2eas_convert_icaltime_to_utcstr(tt, icaltz);
				xmlNewTextChild (exception, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONSTARTTIME, (const xmlChar *) modified);
				free (modified);
			}else{
				xmlNewTextChild (exception, NULL, (const xmlChar*) EAS_NAMESPACE_CALENDAR EAS_ELEMENT_EXCEPTIONSTARTTIME, NULL);
			}
		}

		_ical2eas_process_valarm (icalcomponent_get_first_component (vevent, ICAL_VALARM_COMPONENT), appData);

		if (getenv ("EAS_DEBUG") && (atoi (g_getenv ("EAS_DEBUG")) >= 4)) {
			xmlChar* dump_buffer = NULL;
			int dump_buffer_size = 0;
			xmlIndentTreeOutput = 1;
			xmlDocDumpFormatMemory (doc, &dump_buffer, &dump_buffer_size, 1);
			g_debug ("XML DOCUMENT DUMPED:\n%s", dump_buffer);
			xmlFree (dump_buffer);
		}

		success = TRUE;
	}

 error:
	if (icaltz && icaltz != icaltimezone_get_utc_timezone ())
		icaltimezone_free (icaltz, TRUE);
	if (ical) {
		icalcomponent_free (ical);
	}
	g_free (recurrenceTimes);

	return success;
}

