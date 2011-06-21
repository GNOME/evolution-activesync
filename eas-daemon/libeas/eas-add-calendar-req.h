/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

#ifndef _EAS_ADD_CALENDAR_REQ_H_
#define _EAS_ADD_CALENDAR_REQ_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define EAS_TYPE_ADD_CALENDAR_REQ             (eas_add_calendar_req_get_type ())
#define EAS_ADD_CALENDAR_REQ(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EAS_TYPE_ADD_CALENDAR_REQ, EasAddCalendarReq))
#define EAS_ADD_CALENDAR_REQ_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EAS_TYPE_ADD_CALENDAR_REQ, EasAddCalendarReqClass))
#define EAS_IS_ADD_CALENDAR_REQ(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EAS_TYPE_ADD_CALENDAR_REQ))
#define EAS_IS_ADD_CALENDAR_REQ_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EAS_TYPE_ADD_CALENDAR_REQ))
#define EAS_ADD_CALENDAR_REQ_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EAS_TYPE_ADD_CALENDAR_REQ, EasAddCalendarReqClass))

typedef struct _EasAddCalendarReqClass EasAddCalendarReqClass;
typedef struct _EasAddCalendarReq EasAddCalendarReq;
typedef struct _EasAddCalendarReqPrivate EasAddCalendarReqPrivate;

struct _EasAddCalendarReqClass
{
	EasRequestBaseClass parent_class;
};

struct _EasAddCalendarReq
{
	EasRequestBase parent_instance;
	
	EasAddCalendarReqPrivate * priv;
};

GType eas_add_calendar_req_get_type (void) G_GNUC_CONST;

/** 
 * Create a new calendar request GObject
 *
 * @param[in] account_id
 *	  Unique identifier for a user account.
 * @param[in] sync_key
 *	  The current synchronisation key.
 * @param[in] folder_id
 *	  The identifer for the target server folder.
 * @param[in] serialised_calendar
 *	  A list of strings containing serialised EasItemInfo GObjects.
 * @param[in] flag
 *	  A semaphore used to make the request appear synchronous by waiting for the
 *	  server response. It should be set by the caller immediately after this 
 *	  function is called and cleared in this request's MessageComplete.
 *
 * @return An allocated EasAddCalendarReq GObject or NULL
 */
EasAddCalendarReq *eas_add_calendar_req_new(const gchar* account_id, 
                                            const gchar *sync_key, 
                                            const gchar *folder_id, 
                                            const GSList *serialised_calendar, 
                                            EFlag *flag);

/**
 * Builds the messages required for the request and sends the request to the server.
 *
 * @param[in] self
 *	  The EasAddCalendarReq GObject instance to be Activated.
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean eas_add_calendar_req_Activate(EasAddCalendarReq *self, GError **error);

/**
 * Called from the Soup thread when we have the final response from the server.
 *
 * Responsible for parsing the server response with the help of the message and
 * then clearing the semaphore to allow the caller that activated the request
 * to continue.
 *
 * @param[in] self
 *	  The EasAddCalendarReq GObject instance whose messages are complete.
 * @param[in] doc
 *	  Document tree containing the server's response. This must be freed using
 *	  xmlFreeDoc(). [full transfer]
 * @param[in] error
 *	  A GError code that has been propagated from the server response.
 */
void
eas_add_calendar_req_MessageComplete(EasAddCalendarReq *self, 
                                     xmlDoc* doc, 
                                     GError* error);

/**
 * Reads the server response data into the supplied data structures.
 *
 * Called from the daemon thread after the Soup thread has called MessageComplete
 * releasing the semaphore. Populates the data structures with the results of the
 * parsed server response.
 *
 * @param[in] self
 *	  The EasAddCalendarReq GObject instance whose server response data we accessing.
 * @param[out] ret_sync_key
 *	  The updated synchronisation key from the server, must be freed with 
 *	  g_free(). [full transfer]
 * @param[out] added_items
 *	  A GSList of serialised EasItemInfo objects, caller must free the list 
 *	  contents with g_free() and the list itself with g_slist_free(). [full transfer]
 * @param[out] error
 *	  GError may be NULL if the caller wishes to ignore error details, otherwise
 *	  will be populated with error details if the function returns FALSE. Caller 
 *	  should free the memory with g_error_free() if it has been set. [full transfer]
 *
 * @return TRUE if successful, otherwise FALSE.
 */
gboolean 
eas_add_calendar_req_ActivateFinish (EasAddCalendarReq* self, 
                                     gchar** ret_sync_key, 
                                     GSList** added_items, 
                                     GError **error);


G_END_DECLS

#endif /* _EAS_ADD_CALENDAR_REQ_H_ */
