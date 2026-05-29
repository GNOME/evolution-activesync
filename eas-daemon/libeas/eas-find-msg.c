#include "eas-find-msg.h"
#include "eas-connection-errors.h"
#include <glib.h>

struct _EasFindMsgPrivate {
	gchar *folder_id;
	gchar *query_text;
	gint range_start;
	gint range_end;
};

G_DEFINE_TYPE_WITH_PRIVATE (EasFindMsg, eas_find_msg, EAS_TYPE_MSG_BASE);

static void
eas_find_msg_init (EasFindMsg *object)
{
	EasFindMsgPrivate *priv;
	object->priv = priv = eas_find_msg_get_instance_private (object);
	priv->folder_id = NULL;
	priv->query_text = NULL;
	priv->range_start = 0;
	priv->range_end = 9;
}

static void
eas_find_msg_finalize (GObject *object)
{
	EasFindMsg *msg = (EasFindMsg *) object;
	EasFindMsgPrivate *priv = msg->priv;
	g_free (priv->folder_id);
	g_free (priv->query_text);
	G_OBJECT_CLASS (eas_find_msg_parent_class)->finalize (object);
}

static void
eas_find_msg_class_init (EasFindMsgClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = eas_find_msg_finalize;
}

EasFindMsg *
eas_find_msg_new (const gchar *folder_id,
		  const gchar *query_text,
		  gint range_start,
		  gint range_end)
{
	EasFindMsg *msg = g_object_new (EAS_TYPE_FIND_MSG, NULL);
	EasFindMsgPrivate *priv = msg->priv;
	priv->folder_id = g_strdup (folder_id);
	priv->query_text = g_strdup (query_text);
	priv->range_start = range_start;
	priv->range_end = range_end;
	return msg;
}

xmlDoc *
eas_find_msg_build_message (EasFindMsg *self)
{
	EasFindMsgPrivate *priv = self->priv;
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;
	xmlNode *execute = NULL;
	xmlNode *criterion = NULL;
	xmlNode *query = NULL;
	xmlNode *and_node = NULL;
	xmlNode *collections = NULL;
	xmlNode *collection = NULL;
	xmlNode *options = NULL;
	gchar range[32];

	doc = xmlNewDoc ((xmlChar *) "1.0");
	root = xmlNewDocNode (doc, NULL, (xmlChar *) "Find", NULL);
	xmlDocSetRootElement (doc, root);

	xmlCreateIntSubset (doc,
			    (xmlChar *) "ActiveSync",
			    (xmlChar *) "-//MICROSOFT//DTD ActiveSync//EN",
			    (xmlChar *) "http://www.microsoft.com/");

	xmlNewNs (root, (xmlChar *) "AirSync:", NULL);

	xmlNewChild (root, NULL, (xmlChar *) "SearchId", (xmlChar *) "1");

	execute = xmlNewChild (root, NULL, (xmlChar *) "ExecuteSearch", NULL);
	criterion = xmlNewChild (execute, NULL, (xmlChar *) "MailBoxSearchCriterion", NULL);

	query = xmlNewChild (criterion, NULL, (xmlChar *) "Query", NULL);
	and_node = xmlNewChild (query, NULL, (xmlChar *) "And", NULL);
	xmlNewChild (and_node, NULL, (xmlChar *) "FreeText", (xmlChar *) priv->query_text);

	if (priv->folder_id && *priv->folder_id) {
		collections = xmlNewChild (and_node, NULL, (xmlChar *) "Collections", NULL);
		collection = xmlNewChild (collections, NULL, (xmlChar *) "Collection", NULL);
		xmlNewChild (collection, NULL, (xmlChar *) "CollectionId", (xmlChar *) priv->folder_id);
	}

	options = xmlNewChild (criterion, NULL, (xmlChar *) "Options", NULL);
	snprintf (range, sizeof (range), "%d-%d", priv->range_start, priv->range_end);
	xmlNewChild (options, NULL, (xmlChar *) "Range", (xmlChar *) range);
	xmlNewChild (options, NULL, (xmlChar *) "DeepTraversal", NULL);

	return doc;
}

gboolean
eas_find_msg_parse_response (EasFindMsg *self,
			     xmlDoc *doc,
			     GSList **results,
			     GError **error)
{
	xmlNode *root = NULL;
	xmlNode *node = NULL;
	xmlNode *result_node = NULL;

	(void) self;
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (results != NULL, FALSE);

	*results = NULL;

	if (!doc)
		return TRUE;

	root = xmlDocGetRootElement (doc);
	if (!root)
		return TRUE;

	for (node = root->children; node; node = node->next) {
		if (node->type != XML_ELEMENT_NODE)
			continue;
		if (!g_strcmp0 ((char *) node->name, "Response")) {
			for (result_node = node->children; result_node; result_node = result_node->next) {
				xmlNode *n = NULL;
				gchar *server_id = NULL;
				gchar *collection_id = NULL;
				gchar *class_name = NULL;
				gchar *weighted_rank = NULL;
				gchar *conversation_id = NULL;
				gchar *entry = NULL;

				if (result_node->type != XML_ELEMENT_NODE)
					continue;
				if (g_strcmp0 ((char *) result_node->name, "Result") != 0)
					continue;

				for (n = result_node->children; n; n = n->next) {
					if (n->type != XML_ELEMENT_NODE)
						continue;
					if (!g_strcmp0 ((char *) n->name, "ServerId")) {
						g_free (server_id);
						server_id = (gchar *) xmlNodeGetContent (n);
					} else if (!g_strcmp0 ((char *) n->name, "CollectionId")) {
						g_free (collection_id);
						collection_id = (gchar *) xmlNodeGetContent (n);
					} else if (!g_strcmp0 ((char *) n->name, "Class")) {
						g_free (class_name);
						class_name = (gchar *) xmlNodeGetContent (n);
					} else if (!g_strcmp0 ((char *) n->name, "WeightedRank")) {
						g_free (weighted_rank);
						weighted_rank = (gchar *) xmlNodeGetContent (n);
					} else if (!g_strcmp0 ((char *) n->name, "ConversationId")) {
						g_free (conversation_id);
						conversation_id = (gchar *) xmlNodeGetContent (n);
					}
				}

				entry = g_strdup_printf ("%s|%s|%s|%s|%s",
							 server_id ? : "",
							 collection_id ? : "",
							 class_name ? : "",
							 weighted_rank ? : "",
							 conversation_id ? : "");
				*results = g_slist_append (*results, entry);

				g_free (server_id);
				g_free (collection_id);
				g_free (class_name);
				g_free (weighted_rank);
				g_free (conversation_id);
			}
		}
	}

	return TRUE;
}
