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

#ifndef CAMEL_EAS_TRANSPORT_H
#define CAMEL_EAS_TRANSPORT_H

#include <camel/camel.h>

/* Standard GObject macros */
#define CAMEL_TYPE_EAS_TRANSPORT \
	(camel_eas_transport_get_type ())
#define CAMEL_EAS_TRANSPORT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), CAMEL_TYPE_EAS_TRANSPORT, CamelEasTransport))
#define CAMEL_EAS_TRANSPORT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), CAMEL_TYPE_EAS_TRANSPORT, CamelEasTransportClass))
#define CAMEL_IS_EAS_TRANSPORT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), CAMEL_TYPE_EAS_TRANSPORT))
#define CAMEL_IS_EAS_TRANSPORT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), CAMEL_TYPE_EAS_TRANSPORT))
#define CAMEL_EAS_TRANSPORT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), CAMEL_TYPE_EAS_TRANSPORT, CamelEasTransportClass))

G_BEGIN_DECLS

typedef struct _CamelEasTransport CamelEasTransport;
typedef struct _CamelEasTransportClass CamelEasTransportClass;

struct _CamelEasTransport {
	CamelTransport parent;
	gboolean connected;
};

struct _CamelEasTransportClass {
	CamelTransportClass parent_class;
};

GType camel_eas_transport_get_type (void);

G_END_DECLS

#endif /* CAMEL_EAS_TRANSPORT_H */
