/*
 *  Filename: eas-contact.h
 */

#ifndef _EAS_CONTACT_H_
#define _EAS_CONTACT_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_CONTACT             (eas_contact_get_type ())
#define EAS_CONTACT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CONTACT, EasContact))
#define EAS_CONTACT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CONTACT, EasContactClass))
#define EAS_IS_CONTACT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CONTACT))
#define EAS_IS_CONTACT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CONTACT))
#define EAS_CONTACT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CONTACT, EasContactClass))

typedef struct _EasContactClass EasContactClass;
typedef struct _EasContact EasContact;

struct _EasContactClass
{
	GObjectClass parent_class;
};

struct _EasContact
{
	GObject parent_instance;
};

GType eas_contact_get_type (void) G_GNUC_CONST;

/* TODO:Insert your Contact Interface APIS here*/
gboolean eas_contact_start_sync(EasContact* obj, gint valueIn, GError** error) ;

G_END_DECLS

#endif /* _EAS_CONTACT_H_ */
