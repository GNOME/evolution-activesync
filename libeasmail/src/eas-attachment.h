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

#ifndef _EAS_ATTACHMENT_H_
#define _EAS_ATTACHMENT_H_

#include <glib-object.h>
#include <libxml/xmlstring.h>

G_BEGIN_DECLS

#define EAS_TYPE_ATTACHMENT             (eas_attachment_get_type ())
#define EAS_ATTACHMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ATTACHMENT, EasAttachment))
#define EAS_ATTACHMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ATTACHMENT, EasAttachmentClass))
#define EAS_IS_ATTACHMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ATTACHMENT))
#define EAS_IS_ATTACHMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_ATTACHMENT))
#define EAS_ATTACHMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_ATTACHMENT, EasAttachmentClass))

typedef struct _EasAttachmentClass EasAttachmentClass;
typedef struct _EasAttachment EasAttachment;

struct _EasAttachmentClass {
	GObjectClass parent_class;
};

struct _EasAttachment {
	GObject parent_instance;

	xmlChar *file_reference;		// specifies the location of an item on the server to retrieve
	xmlChar *display_name;			//
	guint estimated_size;			// in bytes
	/*
		guint8  method;					//eg EAS_ATTACHMENT_METHOD_NORMAL
	    gchar *content_id
	    gchar *content_location
	    gboolean is_inline
	*/
};

/*
 enum{
	EAS_ATTACHMENT_METHOD_NORMAL = 1,
	EAS_ATTACHMENT_METHOD_RESERVED1,
	EAS_ATTACHMENT_METHOD_RESERVED2,
	EAS_ATTACHMENT_METHOD_RESERVED3,
	EAS_ATTACHMENT_METHOD_EMBEDDED_EMAIL,
	EAS_ATTACHMENT_METHOD_EMBEDDED_OLE,

	EAS_ATTACHMENT_METHOD_EMBEDDED_MAX
};
*/

GType eas_attachment_get_type (void) G_GNUC_CONST;


/*
Instantiate
*/
EasAttachment *eas_attachment_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_attachment_serialise (EasAttachment *attachment, gchar **result);

G_END_DECLS

#endif /* _EAS_ATTACHMENT_H_ */
