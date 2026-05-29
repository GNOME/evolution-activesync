#include "eas-find-req.h"
#include "eas-find-msg.h"

struct _EasFindReqPrivate {
	EasFindMsg *msg;
	gchar *folder_id;
	gchar *query;
	guint range_start;
	guint range_end;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasFindReq, eas_find_req, EAS_TYPE_REQUEST_BASE);

static void
eas_find_req_init (EasFindReq *object)
{
	EasFindReqPrivate *priv;
	object->priv = priv = eas_find_req_get_instance_private (object);
	priv->msg = NULL;
	priv->folder_id = NULL;
	priv->query = NULL;
	priv->range_start = 0;
	priv->range_end = 9;
	eas_request_base_SetRequestType (&object->parent_instance, EAS_REQ_FIND);
}

static void
eas_find_req_dispose (GObject *object)
{
	EasFindReq *req = EAS_FIND_REQ (object);
	EasFindReqPrivate *priv = req->priv;
	if (priv->msg) {
		g_object_unref (priv->msg);
		priv->msg = NULL;
	}
	G_OBJECT_CLASS (eas_find_req_parent_class)->dispose (object);
}

static void
eas_find_req_finalize (GObject *object)
{
	EasFindReq *req = EAS_FIND_REQ (object);
	EasFindReqPrivate *priv = req->priv;
	g_free (priv->folder_id);
	g_free (priv->query);
	G_OBJECT_CLASS (eas_find_req_parent_class)->finalize (object);
}

static void
eas_find_req_class_init (EasFindReqClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);
	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_find_req_MessageComplete;
	object_class->finalize = eas_find_req_finalize;
	object_class->dispose = eas_find_req_dispose;
}

EasFindReq *
eas_find_req_new (const gchar *account_id,
		  GDBusMethodInvocation *context,
		  const gchar *folder_id,
		  const gchar *query,
		  guint range_start,
		  guint range_end)
{
	EasFindReq *self = g_object_new (EAS_TYPE_FIND_REQ, NULL);
	EasFindReqPrivate *priv = self->priv;

	(void) account_id;
	eas_request_base_SetContext (&self->parent_instance, context);
	priv->folder_id = g_strdup (folder_id);
	priv->query = g_strdup (query);
	priv->range_start = range_start;
	priv->range_end = range_end;

	return self;
}

gboolean
eas_find_req_Activate (EasFindReq *self, GError **error)
{
	EasFindReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv->msg = eas_find_msg_new (priv->folder_id, priv->query,
				      (gint) priv->range_start, (gint) priv->range_end);
	doc = eas_find_msg_build_message (priv->msg);
	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR, EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     "out of memory");
		return FALSE;
	}

	return eas_request_base_SendRequest (parent, "Find", doc, FALSE, error);
}

gboolean
eas_find_req_MessageComplete (EasFindReq *self, xmlDoc *doc, GError *error_in)
{
	gboolean ret = TRUE;
	GError *error = NULL;
	EasFindReqPrivate *priv = self->priv;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);
	GSList *results = NULL;
	GSList *l = NULL;
	guint len = 0;
	const gchar **strv = NULL;
	guint i = 0;

	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_find_msg_parse_response (priv->msg, doc, &results, &error);
	if (!ret)
		goto finish;

	len = g_slist_length (results);
	strv = g_new0 (const gchar *, len + 1);
	for (l = results; l; l = l->next, i++)
		strv[i] = (const gchar *) l->data;

	g_dbus_method_invocation_return_value (eas_request_base_GetContext (parent),
					       g_variant_new ("(^as)", strv));
	g_free (strv);
	g_slist_free_full (results, g_free);

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		g_assert (error != NULL);
		g_dbus_method_invocation_return_gerror (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	}
	return TRUE;
}
