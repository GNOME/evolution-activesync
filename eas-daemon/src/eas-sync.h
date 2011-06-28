/*
 *  Filename: eas-sync.h
 */


#ifndef _EAS_SYNC_H_
#define _EAS_SYNC_H_

#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include "../libeas/eas-connection.h"

G_BEGIN_DECLS

#define EAS_TYPE_SYNC             (eas_sync_get_type ())
#define EAS_SYNC(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_SYNC, EasSync))
#define EAS_SYNC_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_SYNC, EasSyncClass))
#define EAS_IS_SYNC(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_SYNC))
#define EAS_IS_SYNC_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_SYNC))
#define EAS_SYNC_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_SYNC, EasSyncClass))

typedef struct _EasSyncClass EasSyncClass;
typedef struct _EasSync EasSync;
typedef struct _EasSyncPrivate EasSyncPrivate;

struct _EasSyncClass
{
	GObjectClass parent_class;
};

struct _EasSync
{
	GObject parent_instance;

	EasSyncPrivate* priv;
};

GType eas_sync_get_type (void) G_GNUC_CONST;

EasSync* eas_sync_new(void);

EasConnection*  eas_sync_get_eas_connection(EasSync* self);
void eas_sync_set_eas_connection(EasSync* self, EasConnection* easConnObj);


void eas_sync_get_latest_items(EasSync* self,
                               const gchar* account_uid,
                               guint64 type,
                               const gchar* folder_id,
                               const gchar* sync_key,
                               DBusGMethodInvocation* context);

gboolean eas_sync_delete_items(EasSync* self,
                               const gchar* account_uid,
                               const gchar* folder_id,
                               const gchar* sync_key, 
                               const GSList *deleted_items_array,
                               DBusGMethodInvocation* context);

gboolean eas_sync_update_items(EasSync* self,
                               const gchar* account_uid,
                               guint64 type,
                               const gchar* folder_id,
                               const gchar* sync_key, 
                               const gchar **items,
                               DBusGMethodInvocation* context);

gboolean eas_sync_add_items(EasSync* self,
                            const gchar* account_uid,
                            guint64 type,
                            const gchar* folder_id,
                            const gchar* sync_key, 
                            const gchar **items,
                            DBusGMethodInvocation* context);

/*
	sync the entire  folder hierarchy 
*/                            
gboolean eas_sync_sync_folder_hierarchy(EasSync* self,
                                          const gchar* account_uid,
                                          const gchar* sync_key,
                                          DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_SYNC_H_ */
