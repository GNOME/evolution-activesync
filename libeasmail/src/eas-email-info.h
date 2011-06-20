/*
 *  Filename: eas-email-info.h
 */

#ifndef _EAS_EMAIL_INFO_H_
#define _EAS_EMAIL_INFO_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_EMAIL_INFO             (eas_email_info_get_type ())
#define EAS_EMAIL_INFO(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_EMAIL_INFO, EasEmailInfo))
#define EAS_EMAIL_INFO_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_EMAIL_INFO, EasEmailInfoClass))
#define EAS_IS_EMAIL_INFO(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_EMAIL_INFO))
#define EAS_IS_EMAIL_INFO_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_EMAIL_INFO))
#define EAS_EMAIL_INFO_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_EMAIL_INFO, EasEmailInfoClass))

typedef struct _EasEmailInfoClass EasEmailInfoClass;
typedef struct _EasEmailInfo EasEmailInfo;
typedef struct _EasEmailHeader EasEmailHeader;

struct _EasEmailInfoClass
{
	GObjectClass parent_class;
};


#define EAS_EMAIL_READ				0x00000001		// whether email has been read
// Note that ANSWERED_LAST and FORWARDED_LAST are mutually exclusive:
#define EAS_EMAIL_ANSWERED_LAST		0x00000002		// Read-only, set by server. 
#define EAS_EMAIL_FORWARDED_LAST	0x00000004		// Read-only, set by server

#define EAS_VALID_READ		(EAS_EMAIL_READ << 16)  // Validity flags for the above.
#define EAS_VALID_ANSWERED	(EAS_EMAIL_ANSWERED_LAST << 16)
#define EAS_VALID_FORWARDED	(EAS_EMAIL_FORWARDED_LAST << 16)

#define EAS_VALID_IMPORTANCE	(1 << 31)

#define EAS_IMPORTANCE_LOW	0
#define EAS_IMPORTANCE_NORMAL	1
#define EAS_IMPORTANCE_HIGH	2

struct _EasEmailHeader{
	gchar *name;
	gchar *value;
};

struct _EasEmailInfo{
	GObject parent_instance;

	gchar *server_id;		    // from AS server
	GSList *headers;			// list of EasEmailHeaders eg To, From (in the order they're listed in the eas xml)
	GSList *attachments;		// list of EasAttachments this email has. AS calls id the 'file reference'. Immutable
	guint32	flags;			    // bitmap. eg EAS_EMAIL_READ | EAS_EMAIL_ANSWERED TODO not clear where in the EAS xml some of these come from
	GSList *categories;		    // list of categories (strings) that the email belongs to 	
	gsize estimated_size;
	time_t date_received;
	int importance;
	/*
	conversation_id
    conversation_index
	...
	TODO which, if any, of the other fields supplied by exchange should be included?
	*/
	// TODO size, date received (pref as time_t) fields
};

GType eas_email_info_get_type (void) G_GNUC_CONST;

/*
Instantiate
*/
EasEmailInfo *eas_email_info_new();

/*
take the contents of the object and turn it into a null terminated string
*/
gboolean eas_email_info_serialise(EasEmailInfo* self, gchar **result);

/*
populate the object from a string
*/
gboolean eas_email_info_deserialise(EasEmailInfo* self, const gchar *data);


G_END_DECLS

#endif /* _EAS_EMAIL_INFO_H_ */
