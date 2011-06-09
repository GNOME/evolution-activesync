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

#ifndef CAMEL_EAS_PRIVATE_H
#define CAMEL_EAS_PRIVATE_H

/* need a way to configure and save this data, if this header is to
   be installed.  For now, dont install it */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_THREADS
#define CAMEL_EAS_FOLDER_LOCK(f, l) \
	(g_static_mutex_lock(&((CamelEasFolder *)f)->priv->l))
#define CAMEL_EAS_FOLDER_UNLOCK(f, l) \
	(g_static_mutex_unlock(&((CamelEasFolder *)f)->priv->l))
#define CAMEL_EAS_FOLDER_REC_LOCK(f, l) \
	(g_static_rec_mutex_lock(&((CamelEasFolder *)f)->priv->l))
#define CAMEL_EAS_FOLDER_REC_UNLOCK(f, l) \
	(g_static_rec_mutex_unlock(&((CamelEasFolder *)f)->priv->l))
#else
#define CAMEL_EAS_FOLDER_LOCK(f, l)
#define CAMEL_EAS_FOLDER_UNLOCK(f, l)
#define CAMEL_EAS_FOLDER_REC_LOCK(f, l)
#define CAMEL_EAS_FOLDER_REC_UNLOCK(f, l)
#endif

#endif /* CAMEL_EAS_PRIVATE_H */
