#include <stdio.h>

#include "eas-smart-reply-req.h"
#include "eas-smart-reply-msg.h"

struct _EasSmartReplyReqPrivate {
	EasSmartReplyMsg *msg;
	gchar *account_id;
	gchar *client_id;
	gchar *source_folder_id;
	gchar *source_item_id;
	gchar *mime_file;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasSmartReplyReq, eas_smart_reply_req, EAS_TYPE_REQUEST_BASE);

static void
eas_smart_reply_req_init (EasSmartReplyReq *object)
{
	EasSmartReplyReqPrivate *priv;
	object->priv = priv = eas_smart_reply_req_get_instance_private (object);
	priv->msg = NULL;
	priv->account_id = NULL;
	priv->client_id = NULL;
	priv->source_folder_id = NULL;
	priv->source_item_id = NULL;
	priv->mime_file = NULL;
	eas_request_base_SetRequestType (&object->parent_instance, EAS_REQ_SMART_REPLY);
}

static void
eas_smart_reply_req_dispose (GObject *object)
{
	EasSmartReplyReq *req = EAS_SMART_REPLY_REQ (object);
	EasSmartReplyReqPrivate *priv = req->priv;
	if (priv->msg) {
		g_object_unref (priv->msg);
		priv->msg = NULL;
	}
	G_OBJECT_CLASS (eas_smart_reply_req_parent_class)->dispose (object);
}

static void
eas_smart_reply_req_finalize (GObject *object)
{
	EasSmartReplyReq *req = EAS_SMART_REPLY_REQ (object);
	EasSmartReplyReqPrivate *priv = req->priv;
	g_free (priv->account_id);
	g_free (priv->client_id);
	g_free (priv->source_folder_id);
	g_free (priv->source_item_id);
	g_free (priv->mime_file);
	G_OBJECT_CLASS (eas_smart_reply_req_parent_class)->finalize (object);
}

static void
eas_smart_reply_req_class_init (EasSmartReplyReqClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	EasRequestBaseClass *base_class = EAS_REQUEST_BASE_CLASS (klass);
	base_class->do_MessageComplete = (EasRequestBaseMessageCompleteFp) eas_smart_reply_req_MessageComplete;
	object_class->finalize = eas_smart_reply_req_finalize;
	object_class->dispose = eas_smart_reply_req_dispose;
}

EasSmartReplyReq *
eas_smart_reply_req_new (const gchar *account_id,
			 GDBusMethodInvocation *context,
			 const gchar *client_id,
			 const gchar *source_folder_id,
			 const gchar *source_item_id,
			 const gchar *mime_file)
{
	EasSmartReplyReq *self = g_object_new (EAS_TYPE_SMART_REPLY_REQ, NULL);
	EasSmartReplyReqPrivate *priv = self->priv;

	eas_request_base_SetContext (&self->parent_instance, context);
	priv->account_id = g_strdup (account_id);
	priv->client_id = g_strdup (client_id);
	priv->source_folder_id = g_strdup (source_folder_id);
	priv->source_item_id = g_strdup (source_item_id);
	priv->mime_file = g_strdup (mime_file);

	return self;
}

gboolean
eas_smart_reply_req_Activate (EasSmartReplyReq *self, GError **error)
{
	gboolean ret = TRUE;
	EasSmartReplyReqPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	FILE *file = NULL;
	guint64 size = 0;
	size_t result = 0;
	gchar *mime_string = NULL;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	file = fopen (priv->mime_file, "r");
	if (!file) {
		g_set_error (error, EAS_CONNECTION_ERROR, EAS_CONNECTION_ERROR_FILEERROR,
			     "failed to open file %s", priv->mime_file);
		return FALSE;
	}

	fseek (file, 0, SEEK_END);
	size = ftell (file);
	rewind (file);

	mime_string = g_malloc (size + 1);
	if (!mime_string) {
		fclose (file);
		g_set_error (error, EAS_CONNECTION_ERROR, EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     "out of memory");
		return FALSE;
	}
	mime_string[size] = 0;
	result = fread (mime_string, 1, size, file);
	fclose (file);
	if (result != size) {
		g_free (mime_string);
		g_set_error (error, EAS_CONNECTION_ERROR, EAS_CONNECTION_ERROR_FILEERROR,
			     "failed to read file %s", priv->mime_file);
		return FALSE;
	}

	priv->msg = eas_smart_reply_msg_new (priv->account_id, priv->client_id,
					     priv->source_folder_id, priv->source_item_id,
					     mime_string);
	doc = eas_smart_reply_msg_build_message (priv->msg);
	if (!doc) {
		g_set_error (error, EAS_CONNECTION_ERROR, EAS_CONNECTION_ERROR_NOTENOUGHMEMORY,
			     "out of memory");
		return FALSE;
	}

	ret = eas_request_base_SendRequest (parent, "SmartReply", doc, FALSE, error);
	if (!ret)
		g_assert (error == NULL || *error != NULL);
	return ret;
}

gboolean
eas_smart_reply_req_MessageComplete (EasSmartReplyReq *self, xmlDoc *doc, GError *error_in)
{
	gboolean ret = TRUE;
	GError *error = NULL;
	EasSmartReplyReqPrivate *priv = self->priv;
	EasRequestBase *parent = EAS_REQUEST_BASE (&self->parent_instance);

	if (error_in) {
		ret = FALSE;
		error = error_in;
		goto finish;
	}

	ret = eas_smart_reply_msg_parse_response (priv->msg, doc, &error);

finish:
	xmlFreeDoc (doc);
	if (!ret) {
		g_assert (error != NULL);
		g_dbus_method_invocation_return_gerror (eas_request_base_GetContext (parent), error);
		g_error_free (error);
	} else {
		g_dbus_method_invocation_return_value (eas_request_base_GetContext (parent), NULL);
	}
	return TRUE;
}
