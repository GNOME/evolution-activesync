/*
A draft API for the EAS mail client library which clients will use to get email data from Exchange Active Sync
*/

#ifndef EAS_MAIL_H
#define EAS_MAIL_H

#include <glib-object.h>


gboolean DBus_Init();

/* sync items in a specified folder 
Returns lists of Calendar Items. 
Max changes in one sync = 100 (configurable somewhere?)
In the case of created items all fields are filled in.
In the case of deleted items only the serverids are valid. 
In the case of updated items only the serverids, flags and categories are valid.
*/
gboolean getLatest(gchar *sync_key,
    gchar *folder_id,	// folder to sync
	GSList **created,
	GSList **updated,	
	GSList **deleted,
	// guint	window_size,	// max changes we want from server 0..512. AS defaults to 100
	guint	time_filter,		// AS calls filter_type. Eg "sync back 3 days". Required on API?
	gboolean *more_available,	// if there are more changes to sync (window_size exceeded)
	GError **error);


// Delete specified items 
gboolean deleteItems(gchar *sync_key,	// in/out parameter to keep sync in track
		const GSList *items,	// List of guids to delete
		gchar *folder_id,	// folder to sync
		GError **error);

/* 
push updates to server
Note that the only valid changes are to the read flag and to categories (other changes ignored)
*/
gboolean UpdateItems((gchar *sync_key,	// in/out parameter to keep sync in track
		const GSList *items,	// List of guids to delete
		gchar *folder_id,	// folder to sync
		GError **error);


#endif
