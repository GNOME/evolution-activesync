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
	gpointer progress_data;
	CamelService *service;
	EasEmailHandler *handler;
	CamelStream *mimefile, *filtered;
	CamelMimeFilter *filter;
	const gchar *account_uid;
	gchar *fname;
	const gchar *msgid;
	int fd;
	gboolean res;

	EVO3(progress_data = cancellable);
	EVO2(progress_data = camel_operation_registered());

	service = CAMEL_SERVICE (transport);
	account_uid = camel_url_get_param (service->url, "account_uid");

	handler = eas_mail_handler_new (account_uid, error);
	if (!handler)
		return FALSE;

	/* FIXME: Check that From/To/Cc headers match the 'from' and 'recipients'
	   arguments. If not, return an error because Exchange is broken can cannot
	   handle a mismatch (such as with Resent-From/Resent-To) */

	fname = g_build_filename (g_get_tmp_dir(), "eas-out.XXXXXX", NULL);
	fd = g_mkstemp (fname);
	if (fd == -1) {
		g_set_error(error, CAMEL_ERROR, CAMEL_ERROR_GENERIC,
			    _("Failed to create temporary file for sending message"));
		g_free (fname);
		return FALSE;
	}

	mimefile = camel_stream_fs_new_with_fd (fd);

	camel_mime_message_set_best_encoding (message,
					      CAMEL_BESTENC_GET_ENCODING,
					      CAMEL_BESTENC_8BIT);

	filtered = camel_stream_filter_new (mimefile);

	filter = camel_mime_filter_crlf_new (CAMEL_MIME_FILTER_CRLF_ENCODE,
				     CAMEL_MIME_FILTER_CRLF_MODE_CRLF_ONLY);
	camel_stream_filter_add (CAMEL_STREAM_FILTER (filtered), filter);
	g_object_unref (filter);

	EVO3_sync(camel_data_wrapper_write_to_stream)
				(CAMEL_DATA_WRAPPER (message),
				 filtered, EVO3(cancellable,) error);
	camel_stream_flush (filtered, EVO3(cancellable,) error);
	camel_stream_flush (mimefile, EVO3(cancellable,) error);

	g_object_unref (mimefile);
	g_object_unref (filtered);

	msgid = camel_mime_message_get_message_id (message);
	res = eas_mail_handler_send_email(handler, msgid, fname,
					  (EasProgressFn)camel_operation_progress, progress_data, NULL, error);

	unlink (fname);
	g_free (fname);
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
