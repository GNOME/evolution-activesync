/*
 *  Filename: eas-contact.c
 */

#include "eas-contact.h"
#include "eas-contact-stub.h"


G_DEFINE_TYPE (EasContact, eas_contact, G_TYPE_OBJECT);

static void
eas_contact_init (EasContact *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_contact_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_contact_parent_class)->finalize (object);
}

static void
eas_contact_class_init (EasContactClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	object_class->finalize = eas_contact_finalize;
	
	 /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_CONTACT,
                                            &dbus_glib_eas_contact_object_info);
	
}

gboolean eas_contact_start_sync(EasContact* obj, gint valueIn, GError** error)
{
  return TRUE;
}

