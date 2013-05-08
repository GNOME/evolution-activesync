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

#ifndef CAMEL_EAS_STORE_H
#define CAMEL_EAS_STORE_H

#include <camel/camel.h>
#include <libeasmail.h>
#include "camel-eas-store-summary.h"

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_STORE \
	(camel_eas_store_get_type ())
#define CAMEL_EAS_STORE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_STORE, CamelEasStore))
#define CAMEL_EAS_STORE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_STORE, CamelEasStoreClass))
#define CAMEL_IS_EAS_STORE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_STORE))
#define CAMEL_IS_EAS_STORE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_STORE))
#define CAMEL_EAS_STORE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_STORE, CamelEasStoreClass))

#define GW_PARAM_FILTER_INBOX		(1 << 0)

G_BEGIN_DECLS

typedef struct _CamelEasStore CamelEasStore;
typedef struct _CamelEasStoreClass CamelEasStoreClass;
typedef struct _CamelEasStorePrivate CamelEasStorePrivate;

struct _CamelEasStore {
	CamelOfflineStore parent;
	CamelEasStorePrivate *priv;

	CamelEasStoreSummary *summary;
	gchar *storage_path;
};

struct _CamelEasStoreClass {
	CamelOfflineStoreClass parent_class;
};

GType camel_eas_store_get_type (void);
gchar *		eas_get_name	(CamelService *service, gboolean brief);
EasEmailHandler *
		camel_eas_store_get_handler	(CamelEasStore *eas_store);

gboolean	camel_eas_store_connected	(CamelEasStore *store,
						 GCancellable *cancellable,
						 GError **error);

G_END_DECLS

#endif /* CAMEL_EAS_STORE_H */
