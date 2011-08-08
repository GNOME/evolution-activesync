/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8; show-trailing-whitespace: t -*- */
/*
 * ActiveSync core protocol library
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef _EAS_MSG_BASE_H_
#define _EAS_MSG_BASE_H_

#include <glib-object.h>
#include <libxml/xmlreader.h> // xmlDoc

G_BEGIN_DECLS

#define EAS_TYPE_MSG_BASE             (eas_msg_base_get_type ())
#define EAS_MSG_BASE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_MSG_BASE, EasMsgBase))
#define EAS_MSG_BASE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_MSG_BASE, EasMsgBaseClass))
#define EAS_IS_MSG_BASE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_MSG_BASE))
#define EAS_IS_MSG_BASE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_MSG_BASE))
#define EAS_MSG_BASE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_MSG_BASE, EasMsgBaseClass))

typedef struct _EasMsgBaseClass EasMsgBaseClass;
typedef struct _EasMsgBase EasMsgBase;
typedef struct _EasMsgBasePrivate EasMsgBasePrivate;


struct _EasMsgBaseClass {
	GObjectClass parent_class;
};

struct _EasMsgBase {
	GObject parent_instance;

	EasMsgBasePrivate *priv;
};

typedef enum {
	EAS_MSG_BASE = 0, // Not actually valid.
	EAS_SYNC_FOLDER_HIERARCHY,
	EAS_SYNC,

	// Etc

	EAS_LAST
} EasMsgType;

GType eas_msg_base_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* _EAS_MSG_BASE_H_ */
