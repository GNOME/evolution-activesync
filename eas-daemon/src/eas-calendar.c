/*
 *  Filename: eas-calendar.c
 */

#include "eas-calendar.h"
#include "eas-calendar-stub.h"

G_DEFINE_TYPE (EasCalendar, eas_calendar, G_TYPE_OBJECT);

static void
eas_calendar_init (EasCalendar *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_calendar_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_calendar_parent_class)->finalize (object);
}

static void
eas_calendar_class_init (EasCalendarClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_calendar_finalize;
	
    /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_CALENDAR,
                                            &dbus_glib_eas_calendar_object_info);
}


gboolean eas_calendar_start_sync(EasCalendar* obj, gint valueIn, GError** error)
{
  return TRUE;
}


