/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * eas-daemon
 * Copyright (C)  2011 <>
 * 
 * eas-daemon is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * eas-daemon is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EAS_CONNECTION_H_
#define _EAS_CONNECTION_H_

#include <glib-object.h>
#include <libxml/xmlreader.h>
#include "eas-request-base.h"

G_BEGIN_DECLS

#define EAS_TYPE_CONNECTION             (eas_connection_get_type ())
#define EAS_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_CONNECTION, EasConnection))
#define EAS_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_CONNECTION, EasConnectionClass))
#define EAS_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_CONNECTION))
#define EAS_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_CONNECTION))
#define EAS_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_CONNECTION, EasConnectionClass))

typedef struct _EasConnectionClass EasConnectionClass;
typedef struct _EasConnectionPrivate EasConnectionPrivate;
typedef struct _EasConnection EasConnection;


struct _EasConnectionClass
{
	GObjectClass parent_class;
};

struct _EasConnection
{
	GObject parent_instance;

	EasConnectionPrivate* priv;
};

GType eas_connection_get_type (void) G_GNUC_CONST;

void eas_connection_autodiscover (const gchar* email, const gchar* username, const gchar* password, gchar** serverUri, GError** error);

EasConnection* eas_connection_new (const gchar* serverUri, const gchar* username, const gchar* password, GError** error);

// Provisioning APIs
void eas_connection_set_policy_key(EasConnection* self, gchar* policyKey);
void eas_connection_resume_request(EasConnection* self);

void eas_connection_send_request(EasConnection* self, gchar* cmd, xmlDoc* doc, EasRequestBase *request);



G_END_DECLS

#endif /* _EAS_CONNECTION_H_ */
