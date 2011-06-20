
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "eas-account.h"

#include <libedataserver/e-uid.h>

#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>

#include <gconf/gconf-client.h>

#include <libedataserver/e-data-server-util.h>


#define d(x)

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void finalize (GObject *);

G_DEFINE_TYPE (EasAccount, eas_account, G_TYPE_OBJECT)


static void
eas_account_class_init (EasAccountClass *account_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (account_class);

	/* virtual method override */
	object_class->finalize = finalize;

	signals[CHANGED] =
		g_signal_new("changed",
			     G_OBJECT_CLASS_TYPE (object_class),
			     G_SIGNAL_RUN_LAST,
			     G_STRUCT_OFFSET (EasAccountClass, changed),
			     NULL, NULL,
			     g_cclosure_marshal_VOID__INT,
			     G_TYPE_NONE, 1,
			     G_TYPE_INT);
}

static void
eas_account_init (EasAccount *account)
{
	 account->uid = NULL;	 
	 account->serverUri = NULL;
	 account->username = NULL;
	 account->password = NULL; 	 
}


static void
finalize (GObject *object)
{
	EasAccount *account = EAS_ACCOUNT (object);

	g_free (account->uid);
	g_free (account->serverUri);
	g_free (account->username);
	g_free (account->password); 	 	
//	g_free (account->parent_uid);


	G_OBJECT_CLASS (eas_account_parent_class)->finalize (object);
}

/**
 * eas_account_new:
 *
 * Returns: a blank new account which can be filled in and
 * added to an #EasAccountList.
 **/
EasAccount *
eas_account_new (void)
{
	EasAccount *account;

	account = g_object_new (EAS_TYPE_ACCOUNT, NULL);
	account->uid = e_uid_new ();

	return account;
}

/**
 * eas_account_new_from_xml:
 * @xml: an XML account description
 *
 * Returns: a new #EasAccount based on the data in @xml, or %NULL
 * if @xml could not be parsed as valid account data.
 **/
EasAccount *
eas_account_new_from_xml (const gchar *xml)
{
	EasAccount *account;

	account = g_object_new (EAS_TYPE_ACCOUNT, NULL);
	if (!eas_account_set_from_xml (account, xml)) {
		g_object_unref (account);
		return NULL;
	}

	return account;
}

static gboolean
xml_set_bool (xmlNodePtr node, const gchar *name, gboolean *val)
{
	gboolean bool;
	xmlChar *buf;

	if ((buf = xmlGetProp (node, (xmlChar*)name))) {
		bool = (!strcmp ((gchar *)buf, "true") || !strcmp ((gchar *)buf, "yes"));
		xmlFree (buf);

		if (bool != *val) {
			*val = bool;
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
xml_set_int (xmlNodePtr node, const gchar *name, gint *val)
{
	gint number;
	xmlChar *buf;

	if ((buf = xmlGetProp (node, (xmlChar*)name))) {
		number = strtol ((gchar *)buf, NULL, 10);
		xmlFree (buf);

		if (number != *val) {
			*val = number;
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean
xml_set_prop (xmlNodePtr node, const gchar *name, gchar **val)
{
	xmlChar *buf;
	gint res;

	buf = xmlGetProp (node, (xmlChar*)name);
	if (buf == NULL) {
		res = (*val != NULL);
		if (res) {
			g_free (*val);
			*val = NULL;
		}
	} else {
		res = *val == NULL || strcmp (*val, (gchar *)buf) != 0;
		if (res) {
			g_free (*val);
			*val = g_strdup((gchar *)buf);
		}
		xmlFree (buf);
	}

	return res;
}


static gboolean
xml_set_content (xmlNodePtr node, gchar **val)
{
	xmlChar *buf;
	gint res;

	buf = xmlNodeGetContent (node);
	if (buf == NULL) {
		res = (*val != NULL);
		if (res) {
			g_free (*val);
			*val = NULL;
		}
	} else {
		res = *val == NULL || strcmp (*val, (gchar *)buf) != 0;
		if (res) {
			g_free (*val);
			*val = g_strdup((gchar *)buf);
		}
		xmlFree (buf);
	}

	return res;
}


/**
 * eas_account_set_from_xml:
 * @account: an #EasAccount
 * @xml: an XML account description.
 *
 * Changes @account to match @xml.
 *
 * Returns: %TRUE if @account was changed, %FALSE if @account
 * already matched @xml or @xml could not be parsed
 **/
gboolean
eas_account_set_from_xml (EasAccount *account, const gchar *xml)
{
	xmlNodePtr node = NULL;
	xmlDocPtr doc = NULL;
	gboolean changed = FALSE;

	if (!(doc = xmlParseDoc ((xmlChar*)xml)))
		return FALSE;

	node = doc->children;
	if (strcmp ((gchar *)node->name, "account") != 0) {
		xmlFreeDoc (doc);
		return FALSE;
	}

	if (!account->uid)
		xml_set_prop (node, "uid", &account->uid);

	for (node = node->children; node; node = node->next) {
		if (!strcmp ((gchar *)node->name, "uri")) {
			changed |= xml_set_content (node, &account->serverUri);
		} else if (!strcmp ((gchar *)node->name, "username")) {
			changed |= xml_set_content (node, &account->username);
		} else if (!strcmp ((gchar *)node->name, "password")) {
			changed |= xml_set_content (node, &account->password);
		}
	
	}

	xmlFreeDoc (doc);

	g_signal_emit (account, signals[CHANGED], 0, -1);

	return changed;
}


/**
 * eas_account_to_xml:
 * @account: an #EasAccount
 *
 * Returns: an XML representation of @account, which the caller
 * must free.
 **/
gchar *
eas_account_to_xml (EasAccount *account)
{
	xmlNodePtr root, node;
	gchar *tmp = NULL;
	xmlChar *xmlbuf  = NULL;
	xmlDocPtr doc;
	gint n;

	doc = xmlNewDoc ((xmlChar*)"1.0");

	root = xmlNewDocNode (doc, NULL, (xmlChar*)"account", NULL);
	xmlDocSetRootElement (doc, root);
	xmlSetProp (root, (xmlChar*)"uid", (xmlChar*)account->uid);

	node = xmlNewChild (root, NULL, (xmlChar*)"uri", NULL);
	if (account->serverUri)
		xmlNewTextChild (root, NULL, (xmlChar*)"uri", (xmlChar*)account->serverUri);

	node = xmlNewChild (root, NULL, (xmlChar*)"username", NULL);
	if (account->username)
		xmlNewTextChild (root, NULL, (xmlChar*)"username", (xmlChar*)account->username);

	node = xmlNewChild (root, NULL, (xmlChar*)"password", NULL);
	if (account->password)
		xmlNewTextChild (root, NULL, (xmlChar*)"password", (xmlChar*)account->password);
	

	xmlDocDumpMemory (doc, &xmlbuf, &n);
	xmlFreeDoc (doc);

	/* remap to glib memory */
	tmp = g_malloc (n + 1);
	memcpy (tmp, xmlbuf, n);
	tmp[n] = '\0';
	xmlFree (xmlbuf);

	return tmp;
}

/**
 * eas_account_uid_from_xml:
 * @xml: an XML account description
 *
 * Returns: the permanent UID of the account described by @xml
 * (or %NULL if @xml could not be parsed or did not contain a uid).
 * The caller must free this string.
 **/
gchar *
eas_account_uid_from_xml (const gchar *xml)
{
	xmlNodePtr node;
	xmlDocPtr doc;
	gchar *uid = NULL;

	if (!(doc = xmlParseDoc ((xmlChar *)xml)))
		return NULL;

	node = doc->children;
	if (strcmp ((gchar *)node->name, "account") != 0) {
		xmlFreeDoc (doc);
		return NULL;
	}

	xml_set_prop (node, "uid", &uid);
	xmlFreeDoc (doc);

	return uid;
}


#if d(!)0
static void
dump_account (EasAccount *ea)
{
	gchar *xml;

	printf("Account changed\n");
	xml = eas_account_to_xml (ea);
	printf(" ->\n%s\n", xml);
	g_free (xml);
}
#endif
