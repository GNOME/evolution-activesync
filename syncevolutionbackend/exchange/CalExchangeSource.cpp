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

#include "config.h"

#ifdef ENABLE_CALEXCHANGE

#include "CalExchangeSource.h"


#include <event.h>
#include <journal.h>
#include <extendedcalendar.h>
#include <extendedstorage.h>
#include <sqlitestorage.h>
#include <icalformat.h>
#include <memorycalendar.h>

#include <syncevo/SmartPtr.h>

#include <syncevo/declarations.h>
SE_BEGIN_CXX


CalExchangeSource::CalExchangeSource(const SyncSourceParams &params) :
    TestingSyncSource(params)
{
    SyncSourceRevisions::init(this, this, 0, m_operations);
    SyncSourceLogging::init(InitList<std::string>("SUMMARY") + "LOCATION",
                            ", ",
                            m_operations);
#if 0
    // VTODO
    SyncSourceLogging::init(InitList<std::string>("SUMMARY"),
                            ", ",
                            m_operations);
    // VJOURNAL
    SyncSourceLogging::init(InitList<std::string>("SUBJECT"),
                            ", ",
                            m_operations);
#endif


}

CalExchangeSource::~CalExchangeSource()
{

}

void CalExchangeSource::open()
{
//TODO: open connection to exchange server
//TODO: trigger initial sync, and get list of items
//TODO: Populate All Items list - Get from server
}

bool CalExchangeSource::isEmpty()
{
    return false;
}

void CalExchangeSource::close()
{
//TODO: Close connection to exchange server 
}

void CalExchangeSource::enableServerMode()
{
    SyncSourceAdmin::init(m_operations, this);
    SyncSourceBlob::init(m_operations, getCacheDir());
}

bool CalExchangeSource::serverModeEnabled() const
{
    return m_operations.m_loadAdminData;
}

CalExchangeSource::Databases CalExchangeSource::getDatabases()
{
    Databases result;

    //TODO: populate databases list
    return result;
}

void CalExchangeSource::beginSync(const std::string &lastToken, const std::string &resumeToken)
{
    const char *anchor = resumeToken.empty() ? lastToken.c_str() : resumeToken.c_str();
//TODO: Populate, New, Updated, Deleted Items list - get changed from server then put in correct set 
//TODO: Use addItem(LUID, State) where state is  enum State {
//        ANY,
//        NEW,
//        UPDATED,
//       DELETED,
//        MAX
//    };
//TODO: Populate list with ID and data for later extraction ( map(LUID, Data))
//TODO: Populate SyncID with result from server
}

std::string CalExchangeSource::endSync(bool success)
{
    //syncID is kept up to date from responses from the server during sync. just return it here.
    return syncID;
}

void CalExchangeSource::readItem(const string &uid, std::string &item)
{
//TODO: Get item from data map based on uid - populate item.
}

TestingSyncSource::InsertItemResult CalExchangeSource::insertItem(const string &uid, const std::string &item)
{
//TODO:check if uid exists already - if so - update
//TODO:otherwise, insert item and get newUID
//TODO:populate InsertItemResult(uid, revisionstring, updatedflag)


    return InsertItemResult("newUID",
                            "",
                            true);
}


void CalExchangeSource::deleteItem(const string &uid)
{
//TODO: check if uid is in local list - if it isn't then don't do delete
//TODO: otherwise call d-bus api - deleteItem

}

void CalExchangeSource::listAllItems(RevisionMap_t &revisions)
{
//TODO: get revisions from main list
}

std::string CalExchangeSource::getDescription(const string &luid)
{
//TODO: find the item and return the description field - for loggin puposes
}

SE_END_CXX

#endif /* ENABLE_CALEXCHANGE */

#ifdef ENABLE_MODULES
# include "CalExchangeRegister.cpp"
#endif
