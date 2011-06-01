/*
 *  Filename: eas-common.c
 */

#include "eas-common.h"
#include "eas-common-stub.h"


G_DEFINE_TYPE (EasCommon, eas_common, G_TYPE_OBJECT);

static void
eas_common_init (EasCommon *object)
{
	/* TODO: Add initialization code here */
}

static void
eas_common_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (eas_common_parent_class)->finalize (object);
}

static void
eas_common_class_init (EasCommonClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GObjectClass* parent_class = G_OBJECT_CLASS (klass);

    (void)parent_class; // remove warning

	object_class->finalize = eas_common_finalize;
	
	 /* Binding to GLib/D-Bus" */ 
    dbus_g_object_type_install_info(EAS_TYPE_COMMON,
                                            &dbus_glib_eas_common_object_info);

}

gboolean eas_common_start_sync(EasCommon* obj, gint valueIn, GError** error)
{
  return TRUE;
}

