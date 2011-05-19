/*
 *  Filename: eas-common.h
 */

#ifndef _EAS_COMMON_H_
#define _EAS_COMMON_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_COMMON             (eas_common_get_type ())
#define EAS_COMMON(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_COMMON, EasCommon))
#define EAS_COMMON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_COMMON, EasCommonClass))
#define EAS_IS_COMMON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_COMMON))
#define EAS_IS_COMMON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_COMMON))
#define EAS_COMMON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_COMMON, EasCommonClass))

typedef struct _EasCommonClass EasCommonClass;
typedef struct _EasCommon EasCommon;

struct _EasCommonClass
{
	GObjectClass parent_class;
};

struct _EasCommon
{
	GObject parent_instance;
};

GType eas_common_get_type (void) G_GNUC_CONST;

/* TODO:Insert your Common Interface APIS here*/
gboolean eas_common_start_sync(EasCommon* obj, gint valueIn, GError** error) ;

G_END_DECLS

#endif /* _EAS_COMMON_H_ */
