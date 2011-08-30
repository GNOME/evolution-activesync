/*
 * ActiveSync DBus dæmon
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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

struct _EasSyncClass {
	GObjectClass parent_class;
};

struct _EasSync {
	GObject parent_instance;

	EasSyncPrivate* priv;
};

GType eas_sync_get_type (void) G_GNUC_CONST;

EasSync* eas_sync_new (void);

EasConnection*  eas_sync_get_eas_connection (EasSync* self);
void eas_sync_set_eas_connection (EasSync* self, EasConnection* easConnObj);


void eas_sync_get_latest_items (EasSync* self,
				const gchar* account_uid,
				guint64 type,
				const gchar* folder_id,
				const gchar* sync_key,
				DBusGMethodInvocation* context);

gboolean eas_sync_delete_items (EasSync* self,
				const gchar* account_uid,
				const guint64 type,
				const gchar* folder_id,
				const gchar* sync_key,
				const gchar** deleted_items_array,
				DBusGMethodInvocation* context);

gboolean eas_sync_update_items (EasSync* self,
				const gchar* account_uid,
				guint64 type,
				const gchar* folder_id,
				const gchar* sync_key,
				const gchar **items,
				DBusGMethodInvocation* context);

gboolean eas_sync_add_items (EasSync* self,
			     const gchar* account_uid,
			     guint64 type,
			     const gchar* folder_id,
			     const gchar* sync_key,
			     const gchar **items,
			     DBusGMethodInvocation* context);

gboolean
eas_sync_fetch_item (EasSync* self,
		     const gchar* account_uid,
		     const gchar* collection_id,
		     const gchar *server_id,
		     const guint64 type,
		     DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_SYNC_H_ */
