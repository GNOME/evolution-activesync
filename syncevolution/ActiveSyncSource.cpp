/*
 * Copyright (C) 2007-2009 Patrick Ohly <patrick.ohly@gmx.de>
 * Copyright (C) 2009 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef ENABLE_ACTIVESYNC

#include "ActiveSyncSource.h"

#include <stdlib.h>
#include <errno.h>

#include <syncevo/declarations.h>
SE_BEGIN_CXX

void EASItemUnref(EasItemInfo *info) { g_object_unref(&info->parent_instance); }
void GStringUnref(char *str) { g_free(str); }

void ActiveSyncSource::enableServerMode()
{
    SyncSourceAdmin::init(m_operations, this);
    SyncSourceBlob::init(m_operations, getCacheDir());
}
bool ActiveSyncSource::serverModeEnabled() const
{
    return m_operations.m_loadAdminData;
}

ActiveSyncSource::Databases ActiveSyncSource::getDatabases()
{
    Databases result;
    // empty string always selects the default database
    result.push_back(Database("", "", true));
    return result;
}


void ActiveSyncSource::open()
{
    // extract account ID and throw error if missing
    std::string username = m_context->getSyncUsername();

    m_account = username.c_str();
    m_folder = getDatabaseID();

    // create handler
    m_handler.set(eas_sync_handler_new(m_account), "EAS handler");
}

void ActiveSyncSource::close()
{
    // free handler if not done already
    m_handler.set(NULL);
}

void ActiveSyncSource::beginSync(const std::string &lastToken, const std::string &resumeToken)
{
    // claim item node for ids
    m_ids.swap(m_itemNode);

    // incremental sync (non-empty token) or start from scratch
    m_startSyncKey = lastToken;
    if (lastToken.empty()) {
        // slow sync: wipe out cached list of IDs, will be filled anew below
        SE_LOG_DEBUG(this, NULL, "starting slow sync");
        m_ids->clear();
    }

    GErrorCXX gerror;
    EASItemsCXX created, updated;
    EASIdsCXX deleted;
    gchar *buffer;
    if (!eas_sync_handler_get_items(m_handler,
                                    m_startSyncKey.c_str(),
                                    &buffer,
                                    getEasType(),
                                    m_folder.c_str(),
                                    created, updated, deleted,
                                    0,
                                    gerror)) {
        gerror.throwError("reading ActiveSync changes");
    }

    // TODO: Test that we really get an empty token here for an unexpected slow
    // sync. If not, we'll start an incremental sync here and later the engine
    // will ask us for older, unmodified item content which we won't have.


    // populate ID lists and content cache
    BOOST_FOREACH(EasItemInfo *item, created) {
        string luid(item->server_id);
        SE_LOG_DEBUG(this, NULL, "new item %s", luid.c_str());
        addItem(luid, NEW);
        m_ids->setProperty(luid, "1");
        m_items[luid] = item->data;
    }
    BOOST_FOREACH(EasItemInfo *item, updated) {
        string luid(item->server_id);
        SE_LOG_DEBUG(this, NULL, "updated item %s", luid.c_str());
        addItem(luid, UPDATED);
        // m_ids.setProperty(luid, "1"); not necessary, should already exist (TODO: check?!)
        m_items[luid] = item->data;
    }
    BOOST_FOREACH(const char *serverID, deleted) {
        string luid(serverID);
        SE_LOG_DEBUG(this, NULL, "deleted item %s", luid.c_str());
        addItem(luid, DELETED);
        m_ids->removeProperty(luid);
    }

    // now also generate full list of all current items:
    // old items + new (added to m_ids above) - deleted (removed above)
    ConfigProps props;
    m_ids->readProperties(props);
    BOOST_FOREACH(const StringPair &entry, props) {
        const std::string &luid = entry.first;
        SE_LOG_DEBUG(this, NULL, "existing item %s", luid.c_str());
        addItem(luid, ANY);
    }

    // update key
    m_currentSyncKey = buffer;
}

std::string ActiveSyncSource::endSync(bool success)
{
    // store current set of items
    if (!success) {
        m_ids->clear();
    }
    m_ids->flush();

    // let engine do incremental sync next time or start from scratch
    // in case of failure
    return success ? m_currentSyncKey : "";
}

void ActiveSyncSource::deleteItem(const string &luid)
{
    // send delete request
    // TODO (?): batch delete requests
    GSList* items = NULL;
    items = g_slist_prepend(items, (char*)luid.c_str());

    GErrorCXX gerror;
    char *buffer;
    if (!eas_sync_handler_delete_items(m_handler,
                                       m_currentSyncKey.c_str(),
                                       &buffer,
                                       getEasType(),
                                       m_folder.c_str(),
                                       items,
                                       gerror)) {
        gerror.throwError("deleting ActiveSync item");
    }

    // remove from item list
    if (m_ids) {
        m_items.erase(luid);
        m_ids->removeProperty(luid);
    }

    // update key
    m_currentSyncKey = buffer;
}

SyncSourceSerialize::InsertItemResult ActiveSyncSource::insertItem(const std::string &luid, const std::string &data)
{
    SyncSourceSerialize::InsertItemResult res;

    EASItemPtr tmp(eas_item_info_new(), "EasItem");
    EasItemInfo *item = tmp.get();
    if (!luid.empty()) {
        // update
        item->server_id = g_strdup(luid.c_str());
    } else {
        // add
        // TODO: is a local id needed? We don't have one.
    }
    item->data = g_strdup(data.c_str());
    EASItemsCXX items;
    items.push_front(tmp.release());

    GErrorCXX gerror;
    char *buffer;

    // distinguish between update (existing luid)
    // or creation (empty luid)
    if (luid.empty()) {
        // send item to server
        if (!eas_sync_handler_add_items(m_handler,
                                        m_currentSyncKey.c_str(),
                                        &buffer,
                                        getEasType(),
                                        m_folder.c_str(),
                                        items,
                                        gerror)) {
            gerror.throwError("adding ActiveSync item");
        }
        // get new ID from updated item
        res.m_luid = item->server_id;

        // TODO: if someone else has inserted a new calendar item
        // with the same UID as the one we are trying to insert here,
        // what will happen? Does the ActiveSync server prevent
        // adding our own version of the item or does it merge?
        // res.m_merged = ???
    } else {
        // update item on server
        if (!eas_sync_handler_update_items(m_handler,
                                           m_currentSyncKey.c_str(),
                                           &buffer,
                                           getEasType(),
                                           m_folder.c_str(),
                                           items,
                                           gerror)) {
            gerror.throwError("updating ActiveSync item");
        }
        res.m_luid = luid;
    }

    // add/update in cache
    if (m_ids) {
        m_items[res.m_luid] = data;
        m_ids->setProperty(res.m_luid, "1");
    }

    // update key
    m_currentSyncKey = buffer;

    return res;
}

void ActiveSyncSource::readItem(const std::string &luid, std::string &item)
{
    // return straight from cache?
    std::map<std::string, std::string>::iterator it = m_items.find(luid);
    if (it == m_items.end()) {
        // no, must fetch
        EASItemPtr tmp(eas_item_info_new(), "EasItem");
        GErrorCXX gerror;
        if (!eas_sync_handler_fetch_item(m_handler,
                                         m_folder.c_str(),
                                         luid.c_str(),
                                         tmp,
                                         getEasType(),
                                         gerror)) {
            gerror.throwError("reading ActiveSync item");
        }
        item = tmp->data;
    } else {
        item = it->second;
    }
}

SE_END_CXX

#endif /* ENABLE_ACTIVESYNC */

#ifdef ENABLE_MODULES
# include "ActiveSyncSourceRegister.cpp"
#endif
