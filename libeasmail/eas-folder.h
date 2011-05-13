/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * code
 * Copyright (C)  2011 <>
 * 
 * code is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EAS_FOLDER_H_
#define _EAS_FOLDER_H_

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
typedef struct _EasFolderPrivate EasFolderPrivate;

struct _EasFolderClass
{
	GObjectClass parent_class;
};

enum
{
	EAS_FOLDER_TYPE_USER_CREATED_GENERIC = 1,
	EAS_FOLDER_TYPE_DEFAULT_INBOX,	
	EAS_FOLDER_TYPE_DEFAULT_DRAFTS,
	EAS_FOLDER_TYPE_DEFAULT_DELETED_ITEMS,
	EAS_FOLDER_TYPE_DEFAULT_SENT_ITEMS,
	EAS_FOLDER_TYPE_DEFAULT_OUTBOX,
	EAS_FOLDER_TYPE_DEFAULT_TASKS,
	EAS_FOLDER_TYPE_DEFAULT_CALENDAR,
	EAS_FOLDER_TYPE_DEFAULT_CONTACTS,
	EAS_FOLDER_TYPE_DEFAULT_NOTES,
	EAS_FOLDER_TYPE_DEFAULT_JOURNAL,
	EAS_FOLDER_TYPE_DEFAULT_USER_CREATED_MAIL
	//TODO finish filling these in according to MS-ASCMD?	
};

struct _EasFolder
{
	GObject parent_instance;
	EasFolderPrivate *priv; //private data
	
	gchar *parent_id;
	gchar *server_id;		// from AS server. string up to 64 characters
	gchar *display_name;
	guint  type;            // eg EAS_FOLDER_TYPE_DEFAULT_INBOX

};

GType eas_folder_get_type (void) G_GNUC_CONST;

EasFolder *eas_folder_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_folder_serialise(EasFolder* this_g, gchar **result);

/*
populate the object from a string
*/
gboolean eas_folder_deserialise(EasFolder* this_g, const gchar *data);

G_END_DECLS

#endif /* _EAS_FOLDER_H_ */
