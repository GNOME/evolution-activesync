/*
 *  Filename: eas-folder.h
 */

#ifndef _EAS_MAIL_FOLDER_H_
#define _EAS_MAIL_FOLDER_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_FOLDER             (eas_folder_get_type ())
#define EAS_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_FOLDER, EasFolder))
#define EAS_FOLDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_FOLDER, EasFolderClass))
#define EAS_IS_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_FOLDER))
#define EAS_IS_FOLDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_FOLDER))
#define EAS_FOLDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_FOLDER, EasFolderClass))

typedef struct _EasFolderClass EasFolderClass;
typedef struct _EasFolder EasFolder;

struct _EasFolderClass{
	GObjectClass parent_class;
};

enum{
	EAS_FOLDER_TYPE_USER_CREATED_GENERIC = 1,
	EAS_FOLDER_TYPE_DEFAULT_INBOX,
	EAS_FOLDER_TYPE_DEFAULT_DRAFTS,
	EAS_FOLDER_TYPE_DEFAULT_DELETED_ITEMS,
	EAS_FOLDER_TYPE_DEFAULT_SENT_ITEMS,	/* 5 */
	EAS_FOLDER_TYPE_DEFAULT_OUTBOX,
	EAS_FOLDER_TYPE_DEFAULT_TASKS,
	EAS_FOLDER_TYPE_DEFAULT_CALENDAR,
	EAS_FOLDER_TYPE_DEFAULT_CONTACTS,
	EAS_FOLDER_TYPE_DEFAULT_NOTES,		/* 10 */
	EAS_FOLDER_TYPE_DEFAULT_JOURNAL,
	EAS_FOLDER_TYPE_USER_CREATED_MAIL,
	EAS_FOLDER_TYPE_USER_CREATED_CALENDAR,
	EAS_FOLDER_TYPE_USER_CREATED_CONTACTS,
	EAS_FOLDER_TYPE_USER_CREATED_TASKS,	/* 15 */
	EAS_FOLDER_TYPE_USER_CREATED_JOURNAL,
	EAS_FOLDER_TYPE_USER_CREATED_NOTES,
	EAS_FOLDER_TYPE_UNKNOWN,
	EAS_FOLDER_TYPE_RECIPIENT_CACHE,
	EAS_FOLDER_TYPE_MAX
};

struct _EasFolder{
	GObject parent_instance;
	
	gchar *parent_id;
	gchar *folder_id;		// from AS server. string up to 64 characters
	gchar *display_name;
	guint  type;            // eg EAS_FOLDER_TYPE_DEFAULT_INBOX

};

static inline gboolean eas_folder_type_is_mail (int type)
{
	return type == EAS_FOLDER_TYPE_USER_CREATED_MAIL ||
		type == EAS_FOLDER_TYPE_DEFAULT_INBOX ||
		type == EAS_FOLDER_TYPE_DEFAULT_DRAFTS ||
		type == EAS_FOLDER_TYPE_DEFAULT_DELETED_ITEMS ||
		type == EAS_FOLDER_TYPE_DEFAULT_SENT_ITEMS ||
		type == EAS_FOLDER_TYPE_DEFAULT_OUTBOX;
}

GType eas_folder_get_type (void) G_GNUC_CONST;

EasFolder *eas_folder_new();


/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_folder_serialise(EasFolder* folder, gchar **result);

/*
populate the object from a string
*/
gboolean eas_folder_deserialise(EasFolder* folder, const gchar *data);


G_END_DECLS

#endif /* _EAS_MAIL_FOLDER_H_ */
