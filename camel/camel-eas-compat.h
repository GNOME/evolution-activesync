/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Authors: David Woodhouse <dwmw2@infradead.org>
 *
 * Copyright © 2011 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CAMEL_EAS_COMPAT_H
#define CAMEL_EAS_COMPAT_H

/* Fugly as hell, but it does the job... */

#include <camel/camel.h>
#include <libedataserver/eds-version.h>
#include <glib.h>
#include <gio/gio.h>

#if EDS_CHECK_VERSION(2,33,0)
#define EVO2(...)
#define EVO3(...) __VA_ARGS__
#define EVO3_sync(x) x ## _sync
#else
#define EVO2(...) __VA_ARGS__
#define EVO3(...)
#define EVO3_sync(x) x
#endif

gchar *
camel_session_get_password_compat (CamelSession *session,
				   CamelService *service,
				   const gchar *domain,
				   const gchar *prompt,
				   const gchar *item,
				   guint32 flags,
				   GError **error);

CamelService *
camel_session_get_service_compat (CamelSession *session,
				  const gchar *url,
				  CamelProviderType type);

#if ! EDS_CHECK_VERSION(3,1,0)

CamelURL *
camel_service_get_camel_url (CamelService *service);

CamelServiceConnectionStatus
camel_service_get_connection_status (CamelService *service);

#else
gchar *
camel_session_get_storage_path (CamelSession *session,
				CamelService *service,
				GError **error);
#endif

#if ! EDS_CHECK_VERSION(3,3,0)

#define camel_folder_summary_get_unread_count(s) ((s)->unread_count)
#define camel_folder_summary_get_saved_count(s) ((s)->saved_count)
#define camel_folder_summary_get_folder(s) ((s)->folder)
#define camel_folder_summary_get camel_folder_summary_uid

#else

#define CAMEL_URL_HIDE_PASSWORD (0)

#endif

#endif /* CAMEL_EAS_COMPAT_H */
