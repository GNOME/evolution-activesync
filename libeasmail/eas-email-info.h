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

#ifndef _EAS_EMAIL_INFO_H_
#define _EAS_EMAIL_INFO_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_EMAIL_INFO             (eas_email_info_get_type ())
#define EAS_EMAIL_INFO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_EMAIL_INFO, EasEmailInfo))
#define EAS_EMAIL_INFO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_INFO, EasEmailInfoClass))
#define EAS_IS_EMAIL_INFO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_EMAIL_INFO))
#define EAS_IS_EMAIL_INFO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_EMAIL_INFO))
#define EAS_EMAIL_INFO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_EMAIL_INFO, EasEmailInfoClass))

typedef struct _EasEmailInfoClass EasEmailInfoClass;
typedef struct _EasEmailInfo EasEmailInfo;

struct _EasEmailInfoClass
{
	GObjectClass parent_class;
};

#define EAS_EMAIL_READ		0x00000001		// whether email has been read
#define EAS_EMAIL_ANSWERED	0x00000002		// not clear how AS supports answered/forwarded! Read-only
#define EAS_EMAIL_FORWARDED	0x00000004		// Read-only

struct _EasEmailInfo
{
	GObject parent_instance;

	gchar *server_id;		    // from AS server
	gchar *mime_headers;		// for created email will be a string of mime headers separated by crlfs. Immutable
	GSList *attachments;		// list of EasAttachments this email has. AS calls id the 'file reference'. Immutable
	guint8	flags;			    // bitmap. read, answered, forwarded
	GSList *categories;		    // list of categories (strings) that the email belongs to 	
};

GType eas_email_info_get_type (void) G_GNUC_CONST;

/*
Instantiate
*/
EasEmailInfo *eas_email_info_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_email_info_serialise(EasEmailInfo* this_g, gchar **result);

/*
populate the object from a string
*/
gboolean eas_email_info_deserialise(EasEmailInfo* this_g, const gchar *data);


G_END_DECLS

#endif /* _EAS_EMAIL_INFO_H_ */
