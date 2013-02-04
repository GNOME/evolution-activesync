/*
 * ActiveSync client library for email access
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */

#ifndef _EAS_MAIL_FOLDER_H_
#define _EAS_MAIL_FOLDER_H_

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define EAS_TYPE_FOLDER             (eas_folder_get_type ())
#define EAS_FOLDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_FOLDER, EasFolder))
#define EAS_FOLDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_FOLDER, EasFolderClass))
#define EAS_IS_FOLDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_FOLDER))
#define EAS_IS_FOLDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_FOLDER))
#define EAS_FOLDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_FOLDER, EasFolderClass))

typedef struct _EasFolderClass EasFolderClass;
typedef struct _EasFolder EasFolder;

struct _EasFolderClass {
	GObjectClass parent_class;
};

enum {
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

struct _EasFolder {
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
gboolean eas_folder_serialise (EasFolder* folder, gchar **result);

/*
populate the object from a string
*/
gboolean eas_folder_deserialise (EasFolder* folder, const gchar *data);

/*
fetch folders and create a list of EasFolder objects
*/
gboolean eas_folder_get_folder_list (void *client, // Must be a struct eas_gdbus_client*
				     gboolean force_refresh,
				     GSList **folders,
				     GCancellable *cancellable,
				     GError **error);


G_END_DECLS

#endif /* _EAS_MAIL_FOLDER_H_ */
