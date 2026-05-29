#include "eas-connection-errors.h"
#include "eas-smart-forward-msg.h"
#include <wbxml/wbxml.h>
#include <glib.h>

struct _EasSmartForwardMsgPrivate {
	gchar *account_id;
	gchar *client_id;
	gchar *source_folder_id;
	gchar *source_item_id;
	gchar *mime_string;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasSmartForwardMsg, eas_smart_forward_msg, EAS_TYPE_MSG_BASE);

static void
eas_smart_forward_msg_init (EasSmartForwardMsg *object)
{
	EasSmartForwardMsgPrivate *priv;
	object->priv = priv = eas_smart_forward_msg_get_instance_private (object);
	priv->account_id = NULL;
	priv->client_id = NULL;
	priv->source_folder_id = NULL;
	priv->source_item_id = NULL;
	priv->mime_string = NULL;
}

static void
eas_smart_forward_msg_finalize (GObject *object)
{
	EasSmartForwardMsg *msg = (EasSmartForwardMsg *) object;
	EasSmartForwardMsgPrivate *priv = msg->priv;

	g_free (priv->account_id);
	g_free (priv->client_id);
	g_free (priv->source_folder_id);
	g_free (priv->source_item_id);
	g_free (priv->mime_string);

	G_OBJECT_CLASS (eas_smart_forward_msg_parent_class)->finalize (object);
}

static void
eas_smart_forward_msg_class_init (EasSmartForwardMsgClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = eas_smart_forward_msg_finalize;
}

EasSmartForwardMsg *
eas_smart_forward_msg_new (const gchar *account_id,
			   const gchar *client_id,
			   const gchar *source_folder_id,
			   const gchar *source_item_id,
			   gchar *mime_string)
{
	EasSmartForwardMsg *msg = g_object_new (EAS_TYPE_SMART_FORWARD_MSG, NULL);
	EasSmartForwardMsgPrivate *priv = msg->priv;

	priv->account_id = g_strdup (account_id);
	priv->client_id = g_strdup (client_id);
	priv->source_folder_id = g_strdup (source_folder_id);
	priv->source_item_id = g_strdup (source_item_id);
	priv->mime_string = mime_string; // takes ownership

	return msg;
}

xmlDoc *
eas_smart_forward_msg_build_message (EasSmartForwardMsg *self)
{
	EasSmartForwardMsgPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;
	xmlNode *source = NULL;
	gchar *base64data = NULL;

	doc = xmlNewDoc ((xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar *) "SmartForward", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar *) "ActiveSync",
			    (xmlChar *) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar *) "http://www.microsoft.com/");

	xmlNewNs (root, (xmlChar *) "ComposeMail:", NULL);

	xmlNewChild (root, NULL, (xmlChar *) "ClientId", (xmlChar *) priv->client_id);
	xmlNewChild (root, NULL, (xmlChar *) "SaveInSentItems", NULL);

	source = xmlNewChild (root, NULL, (xmlChar *) "Source", NULL);
	xmlNewChild (source, NULL, (xmlChar *) "ItemId", (xmlChar *) priv->source_item_id);
	xmlNewChild (source, NULL, (xmlChar *) "CollectionId", (xmlChar *) priv->source_folder_id);

	if (priv->account_id && *priv->account_id)
		xmlNewChild (root, NULL, (xmlChar *) "AccountId", (xmlChar *) priv->account_id);

	if (priv->mime_string) {
		base64data = g_base64_encode ((const guchar *) priv->mime_string,
					      strlen (priv->mime_string));
		g_free (priv->mime_string);
		priv->mime_string = NULL;
		xmlNewChild (root, NULL, (xmlChar *) "MIME", (xmlChar *) base64data);
		g_free (base64data);
	}

	return doc;
}

gboolean
eas_smart_forward_msg_parse_response (EasSmartForwardMsg *self,
				      xmlDoc *doc,
				      GError **error)
{
	(void) self;
	(void) doc;
	(void) error;
	return TRUE;
}
