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

#ifndef _EAS_COMMON_H_
#define _EAS_COMMON_H_

#include <dbus/dbus-glib.h>
#include <glib-object.h>
#include "../libeas/eas-request-base.h"

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

gboolean eas_common_get_protocol_version (EasCommon *obj,
					  const gchar *account_uid,
					  gchar **ret, GError **error);

/*
	synchronize an email folder. Gets email headers only, not bodies
*/   
gboolean eas_common_sync_folder_items (EasCommon* self,
                               const gchar* account_uid,
                               EasItemType item_type,
                               const gchar* sync_key,
                               const gchar* folder_id,
                               guint filter_type,
                               const gchar** add_items,
                               const gchar** delete_items,                                       
                               const gchar** change_items,
                               DBusGMethodInvocation* context);

G_END_DECLS

#endif /* _EAS_COMMON_H_ */
