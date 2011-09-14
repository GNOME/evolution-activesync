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

#ifndef EAS_MAIL_ERRORS_H
#define EAS_MAIL_ERRORS_H

#include <glib.h>

G_BEGIN_DECLS

GQuark eas_mail_error_quark (void);

#define EAS_MAIL_ERROR (eas_mail_error_quark ())

enum {
	EAS_MAIL_ERROR_NONE,
	EAS_MAIL_ERROR_NOTENOUGHMEMORY,
	EAS_MAIL_ERROR_UNKNOWN,
	EAS_MAIL_ERROR_BADARG
};

G_END_DECLS

#endif	// EAS_MAIL_ERRORS_H
