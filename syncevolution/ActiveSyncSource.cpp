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

#include <syncevo/declarations.h>
SE_BEGIN_CXX

void ActiveSyncSource::enableServerMode()
{
    SyncSourceAdmin::init(m_operations, this);
    SyncSourceBlob::init(m_operations, getCacheDir());
}
bool ActiveSyncSource::serverModeEnabled() const
{
    return m_operations.m_loadAdminData;
}
void ActiveSyncSource::open()
{
    // TODO: extract account ID and throw error if missing
}
void ActiveSyncSource::close()
{
}

void ActiveSyncSource::beginSync(const std::string &lastToken, const std::string &resumeToken)
{
    // TODO: incremental sync (non-empty token) or start from scratch
    // TODO: Test that we really get an empty token here for an unexpected slow
    // sync. If not, we'll start an incremental sync here and later the engine
    // will ask us for older, unmodified item content which we won't have.
}

std::string ActiveSyncSource::endSync(bool success)
{
    // let engine do incremental sync next time or start from scratch
    // in case of failure
    return success ? m_currentSyncKey : "";
}

void ActiveSyncSource::deleteItem(const string &luid)
{
    // TODO: send delete request
    // remove from item list
    // update key
}

SyncSourceSerialize::InsertItemResult ActiveSyncSource::insertItem(const std::string &luid, const std::string &item)
{
    SyncSourceSerialize::InsertItemResult res;

    // distinguish between update (existing luid)
    // or creation (empty luid)
    if (luid.empty()) {
        // TODO: send item to server
        // add to item list
    } else {
        // update item on server
    }
    // update key

    return res;
}

void ActiveSyncSource::readItem(const std::string &luid, std::string &item)
{
    // TODO: return straight from cache
}

SE_END_CXX

#endif /* ENABLE_ACTIVESYNC */

#ifdef ENABLE_MODULES
# include "ActiveSyncSourceRegister.cpp"
#endif
