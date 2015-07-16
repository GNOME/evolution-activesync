/*
 * e-eas-backend.c
 * The collection backend runs in the registry service. It's just a necessary
 * procedure. You don't have to change it unless you really have things to do
 * during the registry.
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
 *		Oliver Luo <lyc.pku.eecs@gmail.com>
 *
 *
 */

#include "e-eas-backend.h"

#define E_EAS_BACKEND_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), E_TYPE_EAS_BACKEND, EEasBackendPrivate))

typedef struct _SyncFoldersClosure SyncFoldersClosure;

struct _EEasBackendPrivate {
	/* Folder ID -> ESource */
	GHashTable *folders;
};

struct _SyncFoldersClosure {
	EEasBackend *backend;
	GSList *folders_created;
	GSList *folders_deleted;
	GSList *folders_updated;
};

G_DEFINE_DYNAMIC_TYPE (
	EEasBackend,
	e_eas_backend,
	E_TYPE_COLLECTION_BACKEND)

static void
eas_backend_dispose (GObject *object)
{
	EEasBackendPrivate *priv;

	priv = E_EAS_BACKEND_GET_PRIVATE (object);

	g_hash_table_remove_all (priv->folders);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (e_eas_backend_parent_class)->dispose (object);
}

static void
eas_backend_finalize (GObject *object)
{
	EEasBackendPrivate *priv;

	priv = E_EAS_BACKEND_GET_PRIVATE (object);
	
	g_hash_table_destroy (priv->folders);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (e_eas_backend_parent_class)->finalize (object);
}

static void
e_eas_backend_class_init (EEasBackendClass *class)
{
	GObjectClass *object_class;
	ECollectionBackendClass *backend_class;

	g_type_class_add_private (class, sizeof (EEasBackendPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = eas_backend_dispose;
	object_class->finalize = eas_backend_finalize;
	backend_class = E_COLLECTION_BACKEND_CLASS (class);
}

static void
e_eas_backend_class_finalize (EEasBackendClass *class)
{
}

static void
e_eas_backend_init (EEasBackend *backend)
{
	backend->priv = E_EAS_BACKEND_GET_PRIVATE (backend);

	backend->priv->folders = g_hash_table_new_full (
		(GHashFunc) g_str_hash,
		(GEqualFunc) g_str_equal,
		(GDestroyNotify) g_free,
		(GDestroyNotify) g_object_unref);
}

void
e_eas_backend_type_register (GTypeModule *type_module)
{
	/* XXX G_DEFINE_DYNAMIC_TYPE declares a static type registration
	 *     function, so we have to wrap it with a public function in
	 *     order to register types from a separate compilation unit. */
	e_eas_backend_register_type (type_module);
}
