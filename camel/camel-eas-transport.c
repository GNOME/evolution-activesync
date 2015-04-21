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

#include <libedataserver/eds-version.h>
#if EDS_CHECK_VERSION(3,17,0)
#include <libemail-engine/libemail-engine.h>
#endif

#include "camel-eas-store.h"
#include "camel-eas-transport.h"

#include "camel-eas-settings.h"

G_DEFINE_TYPE (CamelEasTransport, camel_eas_transport, CAMEL_TYPE_TRANSPORT)

#if EDS_CHECK_VERSION(3,17,0)

static gboolean
eas_transport_sent_folder_is_server_side (CamelService *service,
					  GCancellable *cancellable)
{
	CamelSession *session;
	ESourceRegistry *registry;
	ESource *sibling, *source = NULL;
	gboolean is_server_side = FALSE;

	g_return_val_if_fail (CAMEL_IS_EAS_TRANSPORT (service), FALSE);

	session = camel_service_ref_session (service);
	if (session && E_IS_MAIL_SESSION (session))
		registry = g_object_ref (e_mail_session_get_registry (E_MAIL_SESSION (session)));
	else
		registry = e_source_registry_new_sync (cancellable, NULL);

	if (!registry) {
		g_clear_object (&session);
		return FALSE;
	}

	sibling = e_source_registry_ref_source (registry, camel_service_get_uid (service));
	if (sibling) {
		GList *sources, *siter;
		
		sources = e_source_registry_list_sources (registry, E_SOURCE_EXTENSION_MAIL_SUBMISSION);
		for (siter = sources; siter; siter = siter->next) {
			source = siter->data;

			if (!source || g_strcmp0 (e_source_get_parent (source), e_source_get_parent (sibling)) != 0 ||
			   !e_source_has_extension (source, E_SOURCE_EXTENSION_MAIL_SUBMISSION) ||
			   !e_source_has_extension (source, E_SOURCE_EXTENSION_MAIL_COMPOSITION))
				source = NULL;
			else
				break;
		}

		if (source &&
		    e_source_has_extension (source, E_SOURCE_EXTENSION_MAIL_SUBMISSION) &&
		    e_source_has_extension (source, E_SOURCE_EXTENSION_MAIL_COMPOSITION)) {
			ESourceMailSubmission *subm_extension;
			CamelStore *store = NULL;
			gchar *folder_name = NULL;

			subm_extension = e_source_get_extension (source, E_SOURCE_EXTENSION_MAIL_SUBMISSION);

			if (e_source_mail_submission_get_sent_folder (subm_extension) &&
			    e_mail_folder_uri_parse (session,
			    e_source_mail_submission_get_sent_folder (subm_extension),
			    &store, &folder_name, NULL) & CAMEL_IS_EAS_STORE (store)) {
				CamelEasStore *eas_store = CAMEL_EAS_STORE (store);
				gchar *folder_id_str = NULL;

				folder_id_str = camel_eas_store_summary_get_folder_id_from_name (eas_store->summary, folder_name);
				is_server_side = (folder_id_str != NULL);

				g_free (folder_id_str);
			}

			g_clear_object (&store);
			g_free (folder_name);
		}
		
		g_list_free_full (sources, g_object_unref);
		g_object_unref (sibling);
	}
	
	g_object_unref (registry);
	g_clear_object (&session);

	return is_server_side;
}

#endif

static gboolean
eas_transport_connect_sync (CamelService *service,
                            GCancellable *cancellable,
			    GError **error)
{
	return TRUE;
}

static gchar *
eas_transport_get_name (CamelService *service,
                              gboolean brief)
{
	const gchar *host;
	CamelNetworkSettings *network_settings;
	CamelSettings *settings;

	settings = camel_service_ref_settings (service);

	network_settings = CAMEL_NETWORK_SETTINGS (settings);
	host = camel_network_settings_get_host (network_settings);
	g_object_unref (settings);

	if (brief)
		return g_strdup_printf (
			_("ActiveSync server %s"), host);
	else
		return g_strdup_printf (
			_("ActiveSync mail delivery via %s"), host);
}

#if EDS_CHECK_VERSION(3,17,0)

static gboolean
eas_send_to_sync (CamelTransport *transport,
		  CamelMimeMessage *message,
		  CamelAddress *from,
		  CamelAddress *recipients,
		  gboolean *out_sent_message_saved,
		  GCancellable *cancellable,
		  GError **error)
{
	gpointer progress_data;
	CamelService *service = CAMEL_SERVICE (transport);
	EasEmailHandler *handler;
	CamelStream *mimefile, *filtered;
	CamelMimeFilter *filter;
	const gchar *account_uid;
	gchar *fname;
	const gchar *msgid;
	int fd;
	gboolean res;
	CamelStoreSettings *settings = CAMEL_STORE_SETTINGS (camel_service_ref_settings (service));
	
	progress_data = cancellable;

	account_uid = g_strdup(camel_eas_settings_get_account_uid ((CamelEasSettings *) settings));
	g_object_unref(settings);

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

	camel_data_wrapper_write_to_stream_sync
				(CAMEL_DATA_WRAPPER (message),
				 filtered, cancellable, error);
	camel_stream_flush (filtered, cancellable, error);
	camel_stream_flush (mimefile, cancellable, error);

	g_object_unref (mimefile);
	g_object_unref (filtered);

	if (eas_transport_sent_folder_is_server_side (service, cancellable)) {
		if (out_sent_message_saved)
			*out_sent_message_saved = TRUE;
	}

	msgid = camel_mime_message_get_message_id (message);
	res = eas_mail_handler_send_email(handler, msgid, fname,
					  (EasProgressFn)camel_operation_progress, progress_data, NULL, error);

	unlink (fname);
	g_free (fname);
	g_object_unref (handler);
	return res;
}

#else

static gboolean
eas_send_to_sync (CamelTransport *transport,
		  CamelMimeMessage *message,
		  CamelAddress *from,
		  CamelAddress *recipients,
		  GCancellable *cancellable,
		  GError **error)
{
	gpointer progress_data;
	CamelService *service = CAMEL_SERVICE (transport);
	EasEmailHandler *handler;
	CamelStream *mimefile, *filtered;
	CamelMimeFilter *filter;
	const gchar *account_uid;
	gchar *fname;
	const gchar *msgid;
	int fd;
	gboolean res;
	CamelStoreSettings *settings = CAMEL_STORE_SETTINGS (camel_service_ref_settings (service));
	
	progress_data = cancellable;

	account_uid = g_strdup(camel_eas_settings_get_account_uid ((CamelEasSettings *) settings));
	g_object_unref(settings);

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

	camel_data_wrapper_write_to_stream_sync
				(CAMEL_DATA_WRAPPER (message),
				 filtered, cancellable, error);
	camel_stream_flush (filtered, cancellable, error);
	camel_stream_flush (mimefile, cancellable, error);

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

#endif

static void
camel_eas_transport_class_init (CamelEasTransportClass *class)
{
	CamelServiceClass *service_class;
	CamelTransportClass *transport_class;

	service_class = CAMEL_SERVICE_CLASS (class);
	service_class->settings_type = CAMEL_TYPE_EAS_SETTINGS;
	service_class->connect_sync = eas_transport_connect_sync;
	service_class->get_name = eas_transport_get_name;

	transport_class = CAMEL_TRANSPORT_CLASS (class);
	transport_class->send_to_sync = eas_send_to_sync;
}

static void
camel_eas_transport_init (CamelEasTransport *eas_transport)
{
}
