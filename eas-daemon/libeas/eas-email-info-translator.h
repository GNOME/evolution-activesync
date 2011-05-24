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

#ifndef _EAS_EMAIL_INFO_TRANSLATOR_H_
#define _EAS_EMAIL_INFO_TRANSLATOR_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>

G_BEGIN_DECLS

#define EAS_TYPE_EMAIL_INFO_TRANSLATOR             (eas_email_info_translator_get_type ())
#define EAS_EMAIL_INFO_TRANSLATOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_EMAIL_INFO_TRANSLATOR, EasEmailInfoTranslator))
#define EAS_EMAIL_INFO_TRANSLATOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_INFO_TRANSLATOR, EasEmailInfoTranslatorClass))
#define EAS_IS_EMAIL_INFO_TRANSLATOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_EMAIL_INFO_TRANSLATOR))
#define EAS_IS_EMAIL_INFO_TRANSLATOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_EMAIL_INFO_TRANSLATOR))
#define EAS_EMAIL_INFO_TRANSLATOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_EMAIL_INFO_TRANSLATOR, EasEmailInfoTranslatorClass))

typedef struct _EasEmailInfoTranslatorClass EasEmailInfoTranslatorClass;
typedef struct _EasEmailInfoTranslator EasEmailInfoTranslator;
typedef struct _EasEmailInfoTranslatorPrivate EasEmailInfoTranslatorPrivate;

struct _EasEmailInfoTranslatorClass
{
	GObjectClass parent_class;
};

struct _EasEmailInfoTranslator
{
	GObject parent_instance;
	//EasEmailInfoTranslatorPrivate *priv;
};

GType eas_email_info_translator_get_type (void) G_GNUC_CONST;

// parses the email ApplicationData for an add
gchar *eas_add_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id);

// parses the email ApplicationData for a delete
gchar *eas_delete_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id);

// parses the email ApplicationData for an update
gchar *eas_update_email_appdata_parse_response (EasEmailInfoTranslator* self, xmlNode *node, gchar *server_id);

G_END_DECLS

#endif /* _EAS_EMAIL_INFO_TRANSLATOR_H_ */
