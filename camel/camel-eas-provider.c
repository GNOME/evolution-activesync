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
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include "camel-eas-store.h"
#include "camel-eas-transport.h"

static void add_hash (guint *hash, gchar *s);
static guint eas_url_hash (gconstpointer key);
static gint check_equal (gchar *s1, gchar *s2);
static gint eas_url_equal (gconstpointer a, gconstpointer b);

static CamelProviderConfEntry eas_conf_entries[] = {
	/* override the labels/defaults of the standard settings */

	{ CAMEL_PROVIDER_CONF_SECTION_START, "mailcheck", NULL,
	  N_("Checking for new mail") },
	{ CAMEL_PROVIDER_CONF_CHECKBOX, "check-all", NULL,
	  N_("C_heck for new messages in all folders"), "1" },
	{ CAMEL_PROVIDER_CONF_SECTION_END },

	{ CAMEL_PROVIDER_CONF_SECTION_START, "general", NULL, N_("Options") },
	{ CAMEL_PROVIDER_CONF_CHECKBOX, "filter-inbox", NULL,
	  N_("_Apply filters to new messages in Inbox on this server"), "0" },
	{ CAMEL_PROVIDER_CONF_CHECKBOX, "filter-junk", NULL,
	  N_("Check new messages for J_unk contents"), "0" },
	{ CAMEL_PROVIDER_CONF_CHECKBOX, "filter-junk-inbox", "filter-junk",
	  N_("Only check for Junk messages in the IN_BOX folder"), "0" },
	{ CAMEL_PROVIDER_CONF_CHECKBOX, "stay-synchronized", NULL,
	  N_("Automatically synchroni_ze account locally"), "0" },
	{ CAMEL_PROVIDER_CONF_SECTION_END },

	{ CAMEL_PROVIDER_CONF_END }
};

static CamelProvider eas_provider = {
	"eas",
	N_("ActiveSync"),

	N_("For accessing servers using ActiveSync"),

	"mail",

	CAMEL_PROVIDER_IS_REMOTE | CAMEL_PROVIDER_IS_SOURCE |
	CAMEL_PROVIDER_IS_STORAGE | CAMEL_PROVIDER_IS_EXTERNAL,

	CAMEL_URL_HIDDEN_USER | CAMEL_URL_HIDDEN_AUTH | CAMEL_URL_HIDDEN_HOST,

	eas_conf_entries,

	/* ... */
};

/*TODO support more auth types */
CamelServiceAuthType camel_eas_password_authtype = {
	N_("Password"),

	N_("This option will connect to the ActiveSync server using a "
	   "plaintext password."),

	"",
	TRUE
};

void
camel_provider_module_init(void)
{
	eas_provider.url_hash = eas_url_hash;
	eas_provider.url_equal = eas_url_equal;
	eas_provider.authtypes = g_list_prepend (eas_provider.authtypes, &camel_eas_password_authtype);
	eas_provider.translation_domain = GETTEXT_PACKAGE;

	eas_provider.object_types[CAMEL_PROVIDER_STORE] =  camel_eas_store_get_type ();
	eas_provider.object_types[CAMEL_PROVIDER_TRANSPORT] = camel_eas_transport_get_type ();

	camel_provider_register (&eas_provider);
}

static void
add_hash (guint *hash, gchar *s)
{
	if (s)
		*hash ^= g_str_hash(s);
}

static guint
eas_url_hash (gconstpointer key)
{
	const CamelURL *u = (CamelURL *)key;
	guint hash = 0;

	add_hash (&hash, u->user);
	add_hash (&hash, u->host);
	hash ^= u->port;

	return hash;
}

static gint
check_equal (gchar *s1, gchar *s2)
{
	if (s1 == NULL) {
		if (s2 == NULL)
			return TRUE;
		else
			return FALSE;
	}

	if (s2 == NULL)
		return FALSE;

	return strcmp (s1, s2) == 0;
}

static gint
eas_url_equal (gconstpointer a, gconstpointer b)
{
	const CamelURL *u1 = a, *u2 = b;

	return check_equal (u1->protocol, u2->protocol)
		&& check_equal (u1->user, u2->user)
		&& check_equal (u1->host, u2->host)
		&& u1->port == u2->port;
}
