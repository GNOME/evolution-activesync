/*
 * Copyright (C) 2007-2009 Patrick Ohly <patrick.ohly@gmx.de>
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

#ifndef INCL_CALEXCHANGESYNCSOURCE
#define INCL_CALEXCHANGESYNCSOURCE

#include <syncevo/SyncSource.h>

#ifdef ENABLE_CALEXCHANGE

#include <boost/noncopyable.hpp>

#include <syncevo/declarations.h>
SE_BEGIN_CXX

/**
 * Access contacts stored in Exchange Server.
 *
 * Change Tracking is done by the exchange server.
 * 
 */
class CalExchangeSource : public TestingSyncSource, private SyncSourceAdmin, private SyncSourceBlob, private SyncSourceRevisions, public SyncSourceLogging, private boost::noncopyable
{
  public:
    CalExchangeSource(const SyncSourceParams &params);
    ~CalExchangeSource();

 protected:
    /* implementation of SyncSource interface */
    virtual void open();
    virtual bool isEmpty();
    virtual void close();
    virtual Databases getDatabases();
    virtual void enableServerMode();
    virtual bool serverModeEnabled() const;
    virtual const char *getPeerMimeType() const { return "text/calendar"; }

    /* implementation of SyncSourceSession interface */
    virtual void beginSync(const std::string &lastToken, const std::string &resumeToken);
    virtual std::string endSync(bool success);

    /* implementation of SyncSourceDelete interface */
    virtual void deleteItem(const string &luid);

    /* implementation of SyncSourceSerialize interface */
    virtual const char *getMimeType() const { return "text/calendar"; }
    virtual const char *getMimeVersion() const { return "2.0"; }
    virtual InsertItemResult insertItem(const std::string &luid, const std::string &item);
    virtual void readItem(const std::string &luid, std::string &item);

    /*
     * implementation of SyncSourceRevisions
     *
     * Used for backup/restore (with dummy revision string).
     */
    virtual void listAllItems(RevisionMap_t &revisions);

    /* implementation of SyncSourceLogging */
    virtual std::string getDescription(const string &luid);

 private:

     std::string syncID; 
     std::map<std::string, std::string> m_calData;

    
};

SE_END_CXX

#endif // ENABLE_CALEXCHANGE
#endif // INCL_CALEXCHANGESYNCSOURCE
