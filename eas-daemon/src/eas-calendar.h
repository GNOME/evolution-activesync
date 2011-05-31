/*
 *  Filename: eas-calendar.h
 */


#ifndef _EAS_CALENDAR_H_
#define _EAS_CALENDAR_H_

#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include "../libeas/eas-connection.h"

G_BEGIN_DECLS

#define EAS_TYPE_CALENDAR             (eas_calendar_get_type ())
#define EAS_CALENDAR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CALENDAR, EasCalendar))
#define EAS_CALENDAR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CALENDAR, EasCalendarClass))
#define EAS_IS_CALENDAR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CALENDAR))
#define EAS_IS_CALENDAR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CALENDAR))
#define EAS_CALENDAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CALENDAR, EasCalendarClass))

typedef struct _EasCalendarClass EasCalendarClass;
typedef struct _EasCalendar EasCalendar;
typedef struct _EasCalendarPrivate EasCalendarPrivate;

struct _EasCalendarClass
{
	GObjectClass parent_class;
};

struct _EasCalendar
{
	GObject parent_instance;

	EasCalendarPrivate* priv;
};

GType eas_calendar_get_type (void) G_GNUC_CONST;

EasCalendar* eas_calendar_new(void);

EasConnection*  eas_calendar_get_eas_connection(EasCalendar* self);
void eas_calendar_set_eas_connection(EasCalendar* self, EasConnection* easConnObj);


/* TODO:Insert your Calendar Interface APIS here*/
void eas_calendar_get_latest_calendar_items(EasCalendar* self,
                                          guint64 account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context);

gboolean eas_calendar_delete_calendar_items(EasCalendar* self,
                                    guint64 account_uid,
                                    const gchar* sync_key, 
                                    const GSList *deleted_items_array,
                                    DBusGMethodInvocation* context);

gboolean eas_calendar_update_calendar_items(EasCalendar* self,
                                    guint64 account_uid,
                                    const gchar* sync_key, 
                                    const gchar **calendar_items,
                                    DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_CALENDAR_H_ */
