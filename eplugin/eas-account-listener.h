/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the program; if not, see <http://www.gnu.org/licenses/>
 *
 *
 * Authors:
 *		Srinivasa Ragavan <sragavan@gnome.org>
 *
 *
 */

#ifndef EAS_ACCOUNT_LISTENER_H
#define EAS_ACCOUNT_LISTENER_H

#include <libedataserver/e-account-list.h>
#include<libedataserver/e-source.h>
#include<libedataserver/e-source-list.h>

G_BEGIN_DECLS

#define EAS_TYPE_ACCOUNT_LISTENER            (eas_account_listener_get_type ())
#define EAS_ACCOUNT_LISTENER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ACCOUNT_LISTENER, EasAccountListener))
#define EAS_ACCOUNT_LISTENER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ACCOUNT_LISTENER,  EasAccountListenerClass))
#define EAS_IS_ACCOUNTLISTENER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ACCOUNT_LISTENER))
#define EAS_IS_ACCOUNT_LISTENER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), EAS_TYPE_ACCOUNT_LISTENER))

typedef struct _EasAccountListener EasAccountListener;
typedef struct _EasAccountListenerClass EasAccountListenerClass;
typedef struct _EasAccountListenerPrivate EasAccountListenerPrivate;

struct _EasAccountListener {
       GObject parent;

       EasAccountListenerPrivate *priv;
};

struct _EasAccountListenerClass {
       GObjectClass parent_class;
};

GType                   eas_account_listener_get_type (void);
EasAccountListener *eas_account_listener_new (void);

G_END_DECLS

#endif
