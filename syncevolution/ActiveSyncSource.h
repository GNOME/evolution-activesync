/*
 * Copyright (C) 2007-2009 Patrick Ohly <patrick.ohly@gmx.de>
 * Copyright (C) 2011 Intel Corporation
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

#ifndef INCL_ACTIVESYNCSOURCE
#define INCL_ACTIVESYNCSOURCE

#include <config.h>
#include <syncevo/SyncSource.h>

#ifdef ENABLE_ACTIVESYNC

#include <syncevo/PrefixConfigNode.h>
#include <syncevo/SafeConfigNode.h>
#include <syncevo/SmartPtr.h>
#include <syncevo/GLibSupport.h>

#include <boost/bind.hpp>

#include <string>
#include <map>

#include "libeassync.h"
#include <eas-item-info.h>

#include <syncevo/declarations.h>
SE_BEGIN_CXX


/**
 * Synchronizes contacts, events, tasks and journals with an
 * ActiveSync server. Sub-classes provide the necessary specialization
 * for different data formats.
 *
 * Data is exchanged between ActiveSyncSource, ActiveSync library, and
 * Synthesis engine as vCard 3.0 and iCalendar 2.0 format.  The
 * standard contact and calendar profile is used, with ACTIVESYNC set
 * as sub-rule. This influences how the Synthesis engine converts to
 * and from its internal format. See KDE in
 * src/syncevo/profiles/datatypes/01vcard-profile.xml for an extensive
 * example how that works.
 *
 * Each ActiveSync calendar item is a VCALENDAR which contains all
 * VEVENTs with the same UID. The SyncEvolution/Synthesis engine works
 * with individual VEVENTs per item. This implies that someone has to
 * map between the two concepts. This is done in the derived
 * ActiveSyncCalendarSource, similar to the existing MapSyncSource.
 *
 * Reusing that class was considered, but discarded because
 * MapSyncSource assumes that the class that it wraps uses id/revision
 * string pairs for change tracking, which is not the case for
 * ActiveSync.
 *
 * A sync session is done like this:
 * - The Synthesis sync anchor directly maps to the
 *   ActiveSync sync key.
 * - In SyncSourceSession::beginSync() (inherited from TestingSyncSource),
 *   the ActiveSync library is asked for changes (for a non-empty key)
 *   or all items (empty key).
 * - The returned item IDs are stored in SyncSourceChanges.
 *   Only the server's IDs are used. They map 1:1 with the "luid" in
 *   the SyncSource API and the Synthesis engine.
 *   Because a full list of all existing items is expected,
 *   ActiveSyncSource maintains a list of all known items
 *   in the params.m_nodes.getTrackingNode() that it gets
 *   for that purpose.
 * - The returned item content is cached in a local content cache
 *   and returns items from that when asked to via
 *   SyncSourceSerialize.
 * - As items are added/remove/updated, the content cache, the list
 *   of IDs, and the sync key are updated. The expectation is
 *   that any changes made by other ActiveSync server clients
 *   will be reported when asking for changes based on that updated
 *   key without including changes made by our own client, even
 *   when these changes happen concurrently. This is true for
 *   Exchange 2010 + ActiveSync 12.1.
 *   TODO: write a test program to verify that assumption.
 *   Google + ActiveSync 12.0 do not support this.
 *   TODO: deal with server-side changes reported to us at the
 *   time when we make changes ourselves.
 * - When multiple events are in one item, it can happen that
 *   the event series has to be retrieved individually from the
 *   server. Example:
 *   - nothing changed on server => nothing sent at start of sync,
 *     cache empty
 *   - ActiveSyncCalendarSource must delete a detached recurrence
 *     inside a series
 *   - retrieve series
 *   - remove recurrence
 *   - sent back the updated series
 * - At the end of the sync, the updated ID list is stored and
 *   the updated sync key is returned to the Synthesis engine.
 * - If anything goes wrong, a fatal error is returned to the
 *   Synthesis engine, which then invalidates the sync key and
 *   thus forces a slow sync in the next session.
 *   TODO: investigate more intelligent ways of recovering.
 *   The problem will be that trying again with the original
 *   sync key will return changes made by the client itself
 *   as part of the incomplete sync session.
 *
 * The command line item manipulation operations
 * (--import/export/update/print-items/delete-items) always start a
 * session without a sync key and thus (with the current API) have to
 * download all items before doing anything.
 * TODO: optimize that
 */
class ActiveSyncSource :
    public TestingSyncSource, // == SyncSourceSession, SyncSourceChanges, SyncSourceDelete, SyncSourceSerialize
    // TODO: implement SyncSourceLogging to get nicer debug and command line output
    // virtual public SyncSourceLogging,
    virtual public SyncSourceAdmin,
    virtual public SyncSourceBlob
{
  public:
    ActiveSyncSource(const SyncSourceParams &params) :
        TestingSyncSource(params),
        // Ensure that arbitrary keys can be stored (SafeConfigNode) and
        // that we use a common prefix, so that we can use the key/value store
        // also for other keys if the need ever arises).
        m_itemNode(new PrefixConfigNode("item-",
                                        boost::shared_ptr<ConfigNode>(new SafeConfigNode(params.m_nodes.getTrackingNode())))),
        m_context(params.m_context),
        m_account(0)
        {
            if (!m_context) {
                m_context.reset(new SyncConfig());
            }
        }

    /** sync config used by this instance, never NULL */
    SyncConfig &getSyncConfig() { return *m_context; }

 protected:
    /* partial implementation of SyncSource */
    virtual void enableServerMode();
    virtual bool serverModeEnabled() const;
    virtual Databases getDatabases();
    virtual void open();
    virtual void close();
    virtual std::string getPeerMimeType() const { return getMimeType(); }

    /* implementation of SyncSourceSession */
    virtual void beginSync(const std::string &lastToken, const std::string &resumeToken);
    virtual std::string endSync(bool success);

    /* implementation of SyncSourceDelete */
    virtual void deleteItem(const string &luid);

    /* partial implementation of SyncSourceSerialize */
    virtual std::string getMimeType() const = 0;
    virtual std::string getMimeVersion() const = 0;
    virtual InsertItemResult insertItem(const std::string &luid, const std::string &item);
    virtual void readItem(const std::string &luid, std::string &item);

    /** to be provided by derived class */
    virtual EasItemType getEasType() const = 0;

 protected:
    EasSyncHandler *getHandler() { return m_handler.get(); }
    std::string getFolder() { return m_folder; }
    std::string getStartSyncKey() { return m_startSyncKey; }
    void setStartSyncKey(const std::string &startSyncKey) { m_startSyncKey = startSyncKey; }
    std::string getCurrentSyncKey() { return m_currentSyncKey; }
    void setCurrentSyncKey(const std::string &currentSyncKey) { m_currentSyncKey = currentSyncKey; }

    boost::shared_ptr<ConfigNode> m_itemNode;

 private:
    /** "source-config@<context>" instance which holds our username == ActiveSync account ID */
    boost::shared_ptr<SyncConfig> m_context;

    /** account ID for libeas, must be set in "username" config property */
    const char* m_account;

    /** folder ID for libeas, optionally set in "database" config property */
    std::string m_folder;

    /** smart pointer holding reference to EasSyncHandler during session */
    eptr<EasSyncHandler, GObject> m_handler;

    /** original sync key, set when session starts */
    std::string m_startSyncKey;

    /** current sync key, set when session starts and updated as changes are made */
    std::string m_currentSyncKey;

    /**
     * server-side IDs of all items, updated as changes are reported and/or are made;
     * NULL if not using change tracking
     */
    boost::shared_ptr<ConfigNode> m_ids;

    /**
     * cache of all items, filled at begin of session and updated as
     * changes are made (if doing change tracking)
     */
    std::map<std::string, std::string> m_items;
};

class ActiveSyncContactSource : public ActiveSyncSource
{
 public:
    ActiveSyncContactSource(const SyncSourceParams &params) :
    ActiveSyncSource(params)
    {}

 protected:
    /* partial implementation of SyncSourceSerialize */
    virtual std::string getMimeType() const { return "text/vcard"; }
    virtual std::string getMimeVersion() const { return "3.0"; }

    EasItemType getEasType() const { return EAS_ITEM_CONTACT; }

#if 0 // currently disabled, and thus using the same conversion as the Evolution backend
    void getSynthesisInfo(SynthesisInfo &info,
                          XMLConfigFragments &fragments)
    {
        TrackingSyncSource::getSynthesisInfo(info, fragments);

        /** enable the ActiveSync X- extensions in the Synthesis<->backend conversion */
        info.m_backendRule = "ACTIVESYNC";

        /*
         * Disable the default VCARD_BEFOREWRITE_SCRIPT_EVOLUTION.
         * If any KDE-specific transformations via such a script
         * are needed, it can be named here and then defined by appending
         * to the fragments.
         */
        info.m_beforeWriteScript = ""; // "$VCARD_BEFOREWRITE_SCRIPT_KDE;";
        // fragments.m_datatypes["VCARD_BEFOREWRITE_SCRIPT_KDE"] = "<macro name=\"VCARD_BEFOREWRITE_SCRIPT_KDE\"><![DATA[ ... ]]></macro>";
    }
#endif
};

/**
 * used for all iCalendar 2.0 items (events, todos, journals)
 */
class ActiveSyncCalFormatSource : public ActiveSyncSource
{
    EasItemType m_type;

 public:
    ActiveSyncCalFormatSource(const SyncSourceParams &params, EasItemType type) :
    ActiveSyncSource(params),
    m_type(type)
    {}

 protected:
    /* partial implementation of SyncSourceSerialize */
    virtual std::string getMimeType() const { return "text/calendar"; }
    virtual std::string getMimeVersion() const { return "2.0"; }

    EasItemType getEasType() const { return m_type; }
};

void EASItemUnref(EasItemInfo *info);

/** non-copyable list of EasItemInfo pointers, owned by list */
typedef GListCXX<EasItemInfo, GSList, EASItemUnref> EASItemsCXX;

void GStringUnref(char *str);

/** non-copyable list of strings, owned by list */
typedef GListCXX<char, GSList, GStringUnref> EASIdsCXX;

/** non-copyable smart pointer to an EasItemInfo, unrefs when going out of scope */
typedef eptr<EasItemInfo, GObject> EASItemPtr;

SE_END_CXX

#endif // ENABLE_ACTIVESYNC
#endif // INCL_ACTIVESYNCSOURCE
