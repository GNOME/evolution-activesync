/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * intelgit
 * Copyright (C)  2011 <>
 * 
 */

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

// C'tor
EasAddCalendarReq *eas_add_calendar_req_new(guint64 account_id, const gchar *sync_key, const gchar *folder_id, const GSList *serialised_calendar, EFlag *flag);

// start async request
void eas_add_calendar_req_Activate(EasAddCalendarReq *self);

// async request completed
void eas_add_calendar_req_MessageComplete(EasAddCalendarReq *self, xmlDoc* doc, GError** error);

// results returned to client
void eas_add_calendar_req_ActivateFinish (EasAddCalendarReq* self, gchar** ret_sync_key, GSList** added_items, GError **error);


G_END_DECLS

#endif /* _EAS_ADD_CALENDAR_REQ_H_ */
