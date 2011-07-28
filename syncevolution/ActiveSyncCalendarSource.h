/*
 * Copyright (C) 2010,2011 Intel Corporation
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

#ifndef INCL_ACTIVESYNCCALENDARSOURCE
#define INCL_ACTIVESYNCCALENDARSOURCE

#include <config.h>

#ifdef ENABLE_ACTIVESYNC

#include "ActiveSyncSource.h"
#include <syncevo/MapSyncSource.h>
#include <syncevo/eds_abi_wrapper.h>
#include <syncevo/SmartPtr.h>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <syncevo/declarations.h>
SE_BEGIN_CXX

/**
 * Similar to CalDAVSource (and partly copied from it): implements
 * operations with one VEVENT per item in terms of operations
 * on items which bundle all VEVENTs with the same UID in one item.
 *
 * It works by keeping all active items as VCALENDAR icalcomponent in
 * memory and updating that when asked to add/update/remove VEVENTs.
 * The update VCALENDAR items are then stored via the base class.
 *
 * Terminology:
 * - easid = davLUID in CalDAVSource =
             ActiveSync item ID used in base ActiveSyncSource and by ActiveSync peer,
 *           1:1 mapping to uid
 * - uid = mainid in MapSyncSource = the UID shared by multiple VEVENTs
 * - rid = subid in MapSyncSource = RECURRENCE-ID, turned into a simple string by Event::getRid()
 * - luid = easid/rid concatenated by createLUID() and split with splitLUID()
 */
class ActiveSyncCalendarSource : public ActiveSyncCalFormatSource
{
 public:
    ActiveSyncCalendarSource(const SyncSourceParams &params, EasItemType type);

    // override operations in base class to work with luid == easid/rid instead
    // of just easid
    virtual std::string getDescription(const string &luid);
    virtual void beginSync(const std::string &lastToken, const std::string &resumeToken);
    virtual std::string endSync(bool success);
    virtual void deleteItem(const string &luid);
    virtual InsertItemResult insertItem(const std::string &luid, const std::string &item);
    virtual void readItem(const std::string &luid, std::string &item);

    /** split luid into uid (first) and rid (second) */
    static StringPair splitLUID(const std::string &luid);

 private:
    /** compose luid from mainid and subid */
    static std::string createLUID(const std::string &uid, const std::string &rid);

    /** escape / in uid with %2F, so that splitMainIDValue() and splitLUID() can use / as separator */
    static StringEscape m_escape;

    /**
     * Information about each merged item.
     */
    class Event : boost::noncopyable {
    public:
        /** the ActiveSync ID */
        std::string m_easid;

        /** the iCalendar 2.0 UID */
        std::string m_uid;

        /**
         * the list of simplified RECURRENCE-IDs (without time zone,
         * see icalTime2Str()), empty string for VEVENT without
         * RECURRENCE-ID
         */
        std::set<std::string> m_subids;

        /**
         * parsed VCALENDAR component representing the current
         * state of the item as it exists on the WebDAV server,
         * must be kept up-to-date as we make changes, may be NULL
         */
        eptr<icalcomponent> m_calendar;

        /** date-time as string, without time zone */
        static std::string icalTime2Str(const icaltimetype &tt);

        /** RECURRENCE-ID, empty if none */
        static std::string getSubID(icalcomponent *icomp);

        /** UID, empty if none */
        static std::string getUID(icalcomponent *icomp);
        static void setUID(icalcomponent *icomp, const std::string &uid);
    };

    /**
     * A cache of information about each merged item. Maps from
     * easid to Event.
     */
    class EventCache : public std::map<std::string, boost::shared_ptr<Event> >
    {
      public:
        EventCache() : m_initialized(false) {}
        bool m_initialized;

        iterator findByUID(const std::string &uid);
    } m_cache;

    Event &findItem(const std::string &easid);
    Event &loadItem(const std::string &easid);
    Event &loadItem(Event &event);

    /**
     * create event (if necessary) and populate based on given iCalendar 2.0 VCALENDAR
     */
    Event &setItemData(const std::string &easid, const std::string &data);

    /**
     * On-disk representation of m_cache (without the item data).
     * Format same as in MapSyncSource (code copied, refactor!).
     */
    boost::shared_ptr<ConfigNode> m_trackingNode;
};

SE_END_CXX

#endif // ENABLE_ACTIVESYNC
#endif // INCL_ACTIVESYNCCALENDARSOURCE
