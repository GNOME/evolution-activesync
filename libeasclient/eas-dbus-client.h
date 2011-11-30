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

#ifndef EAS_DBUS_CLIENT_H
#define EAS_DBUS_CLIENT_H

#include <glib.h>
#include <gio/gio.h>
#include "../eas-daemon/src/activesyncd-common-defs.h"

typedef void (*EasProgressFn) (gpointer object, gint percent);

struct eas_gdbus_client {
	GDBusConnection *connection;
	gchar* account_uid;
	GHashTable *progress_fns_table;
	GMutex *progress_lock;
#if GLIB_CHECK_VERSION (2,31,0)
	GMutex _mutex;
#endif
};

struct _EasProgressCallbackInfo {
	EasProgressFn progress_fn;
	gpointer progress_data;
	guint percent_last_sent;

	guint handler_id;	//
	GCancellable cancellable;	//

};

typedef struct _EasProgressCallbackInfo EasProgressCallbackInfo;


void
eas_gdbus_client_destroy (struct eas_gdbus_client *client);

gboolean
eas_gdbus_client_init (struct eas_gdbus_client *client,
		       const gchar *account_uid, GError **error);

gboolean
eas_gdbus_call (struct eas_gdbus_client *client, const gchar *object,
		const gchar *interface, const gchar *method,
		EasProgressFn progress_fn, gpointer progress_data,
		const gchar *in_params, const gchar *out_params,
		GCancellable *cancellable, GError **error, ...);

gboolean
eas_gdbus_call_finish (struct eas_gdbus_client *client, GAsyncResult *result,
		       guint cancel_serial, const gchar *out_params,
		       va_list *ap, GError **error);

#endif /* EAS_DBUS_CLIENT_H */
