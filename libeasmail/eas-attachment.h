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

#ifndef _EAS_ATTACHMENT_H_
#define _EAS_ATTACHMENT_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_ATTACHMENT             (eas_attachment_get_type ())
#define EAS_ATTACHMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ATTACHMENT, EasAttachment))
#define EAS_ATTACHMENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ATTACHMENT, EasAttachmentClass))
#define EAS_IS_ATTACHMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ATTACHMENT))
#define EAS_IS_ATTACHMENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_ATTACHMENT))
#define EAS_ATTACHMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_ATTACHMENT, EasAttachmentClass))

typedef struct _EasAttachmentClass EasAttachmentClass;
typedef struct _EasAttachment EasAttachment;

struct _EasAttachmentClass
{
	GObjectClass parent_class;
};

struct _EasAttachment
{
	GObject parent_instance;

	gchar *file_reference;		
	guint estimated_size;
	gchar *display_name;
	// method;
	// inline;	
};

GType eas_attachment_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _EAS_ATTACHMENT_H_ */
