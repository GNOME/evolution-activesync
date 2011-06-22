
#ifndef _EAS_REQUEST_BASE_H_
#define _EAS_REQUEST_BASE_H_

#include <glib-object.h>
#include <libsoup/soup.h>
#include <libxml/xmlreader.h> // xmlDoc
#include <libedataserver/e-flag.h>
#include "eas-connection.h"
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#define EAS_TYPE_REQUEST_BASE             (eas_request_base_get_type ())
#define EAS_REQUEST_BASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_REQUEST_BASE, EasRequestBase))
#define EAS_REQUEST_BASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_REQUEST_BASE, EasRequestBaseClass))
#define EAS_IS_REQUEST_BASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_REQUEST_BASE))
#define EAS_IS_REQUEST_BASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_REQUEST_BASE))
#define EAS_REQUEST_BASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_REQUEST_BASE, EasRequestBaseClass))

typedef struct _EasRequestBaseClass EasRequestBaseClass;
typedef struct _EasRequestBase EasRequestBase;
typedef struct _EasRequestBasePrivate EasRequestBasePrivate;


struct _EasRequestBaseClass
{
	GObjectClass parent_class;
};

struct _EasRequestBase
{
	GObject parent_instance;
	
	EasRequestBasePrivate* priv;
};

typedef enum {
	EAS_REQ_BASE = 0,
    EAS_REQ_PROVISION,
	EAS_REQ_SYNC_FOLDER_HIERARCHY,
	EAS_REQ_SYNC,
	EAS_REQ_SEND_EMAIL,
	EAS_REQ_DELETE_MAIL,
    EAS_REQ_GET_EMAIL_BODY,
	EAS_REQ_UPDATE_MAIL,
    EAS_REQ_GET_EMAIL_ATTACHMENT,
    EAS_REQ_UPDATE_CALENDAR,
	EAS_REQ_ADD_CALENDAR,
	EAS_REQ_MOVE_EMAIL,
	//TODO: add all other requests here
	
	EAS_REQ_LAST
} EasRequestType;

typedef enum {
	EAS_ITEM_FOLDER=0,
	EAS_ITEM_MAIL,
	EAS_ITEM_CALENDAR,    /**< iCalendar 2.0 VEVENT */
	EAS_ITEM_CONTACT,     /**< vCard 3.0 contact */
        EAS_ITEM_TODO,        /**< iCalendar 2.0 VTODO */
        EAS_ITEM_JOURNAL,     /**< iCalendar 2.0 VJOURNAL */

	//TODO: add all other items here
	
	EAS_ITEM_LAST
}EasItemType;

GType eas_request_base_get_type (void) G_GNUC_CONST;

void eas_request_base_Activate (EasRequestBase *self) __attribute__((noreturn));
void eas_request_base_MessageComplete (EasRequestBase *self, xmlDoc* doc) __attribute__((noreturn));

EasRequestType eas_request_base_GetRequestType(EasRequestBase* self);
void eas_request_base_SetRequestType(EasRequestBase* self, EasRequestType type);

struct _EasConnection* eas_request_base_GetConnection(EasRequestBase* self);
void eas_request_base_SetConnection(EasRequestBase* self, struct _EasConnection* connection);

SoupMessage *eas_request_base_GetSoupMessage(EasRequestBase* self);
void eas_request_base_SetSoupMessage(EasRequestBase* self, SoupMessage *soup_message);

DBusGMethodInvocation *eas_request_base_GetContext (EasRequestBase* self);
void eas_request_base_SetContext(EasRequestBase* self, DBusGMethodInvocation* context);

EFlag *eas_request_base_GetFlag (EasRequestBase* self);
void eas_request_base_SetFlag(EasRequestBase* self, EFlag* flag);

G_END_DECLS

#endif /* _EAS_REQUEST_BASE_H_ */
