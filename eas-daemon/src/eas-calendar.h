/*
 *  Filename: eas-calendar.h
 */


#ifndef _EAS_CALENDAR_H_
#define _EAS_CALENDAR_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_CALENDAR             (eas_calendar_get_type ())
#define EAS_CALENDAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CALENDAR, EasCalendar))
#define EAS_CALENDAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CALENDAR, EasCalendarClass))
#define EAS_IS_CALENDAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CALENDAR))
#define EAS_IS_CALENDAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CALENDAR))
#define EAS_CALENDAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CALENDAR, EasCalendarClass))

typedef struct _EasCalendarClass EasCalendarClass;
typedef struct _EasCalendar EasCalendar;

struct _EasCalendarClass
{
	GObjectClass parent_class;
};

struct _EasCalendar
{
	GObject parent_instance;
};

GType eas_calendar_get_type (void) G_GNUC_CONST;

/* TODO:Insert your Calendar Interface APIS here*/
gboolean eas_calendar_start_sync(EasCalendar* obj, gint valueIn, GError** error) ;

G_END_DECLS

#endif /* _EAS_CALENDAR_H_ */
