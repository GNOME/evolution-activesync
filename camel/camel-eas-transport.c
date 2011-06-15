/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Authors: David Woodhouse <dwmw2@infradead.org>
 *
 * Copyright © 2010-2011 Intel Corporation (www.intel.com)
 * 
 * Based on GroupWise/EWS code which was
 *  Copyright © 1999-2008 Novell, Inc. (www.novell.com)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n-lib.h>

#include <camel-eas-compat.h>

#include "camel-eas-store.h"
#include "camel-eas-transport.h"

G_DEFINE_TYPE (CamelEasTransport, camel_eas_transport, CAMEL_TYPE_TRANSPORT)

static gboolean
eas_transport_connect_sync (CamelService *service,
                            EVO3(GCancellable *cancellable,)
			    GError **error)
{
	return TRUE;
}

static gchar *
eas_transport_get_name (CamelService *service,
                              gboolean brief)
{
	if (brief)
		return g_strdup_printf (
			_("Exchange server %s"),
			service->url->host);
	else
		return g_strdup_printf (
			_("Exchange mail delivery via %s"),
			service->url->host);
}

static gboolean
eas_send_to_sync (CamelTransport *transport,
		  CamelMimeMessage *message,
		  CamelAddress *from,
		  CamelAddress *recipients,
		  EVO3(GCancellable *cancellable,)
		  GError **error)
{
	EVO2(GCancellable *cancellable = NULL;)
	CamelService *service;
	EasEmailHandler *handler;
	const gchar* account_uid;
	gboolean res;

	service = CAMEL_SERVICE (transport);
	account_uid = camel_url_get_param (service->url, "account_uid");

	handler = eas_mail_handler_new (account_uid);
	if (!handler) {
		g_set_error (error, CAMEL_SERVICE_ERROR,
			     CAMEL_SERVICE_ERROR_NOT_CONNECTED,
			     _("Service not connected"));
		return FALSE;
	}

  //	res = camel_eas_utils_create_mime_message (handler, "SendOnly", NULL,
  //						   message, 0, from,
  //						   NULL, NULL,
  //						   cancellable, error);
	g_object_unref (handler);
	return res;
}

static void
camel_eas_transport_class_init (CamelEasTransportClass *class)
{
	CamelServiceClass *service_class;
	CamelTransportClass *transport_class;

	service_class = CAMEL_SERVICE_CLASS (class);
	service_class->EVO3_sync(connect) = eas_transport_connect_sync;
	service_class->get_name = eas_transport_get_name;

	transport_class = CAMEL_TRANSPORT_CLASS (class);
	transport_class->EVO3_sync(send_to) = eas_send_to_sync;
}

static void
camel_eas_transport_init (CamelEasTransport *eas_transport)
{
}
