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

#include "config.h"

#ifdef ENABLE_ACTIVESYNC

// include first, it sets HANDLE_LIBICAL_MEMORY for us
#include <syncevo/icalstrdup.h>

#include "ActiveSyncCalendarSource.h"
#include <syncevo/GLibSupport.h>

#include <syncevo/declarations.h>
SE_BEGIN_CXX


ActiveSyncCalendarSource::ActiveSyncCalendarSource(const SyncSourceParams &params, EasItemType type) :
    ActiveSyncCalFormatSource(params, type)
{
}

void ActiveSyncCalendarSource::beginSync(const std::string &lastToken, const std::string &resumeToken)
{
    // claim item node for our change tracking
    m_trackingNode.swap(m_itemNode);

    // incremental sync (non-empty token) or start from scratch
    setStartSyncKey(lastToken);
    if (lastToken.empty()) {
        // slow sync: wipe out cached list of IDs, will be filled anew below
        SE_LOG_DEBUG(this, NULL, "starting slow sync");
        m_trackingNode->clear();
    } else {
        // re-populate cache from storage, without any item data
        ConfigProps props;
        m_trackingNode->readProperties(props);
        BOOST_FOREACH(const StringPair &prop, props) {
            const std::string &easid = prop.first;
            const std::string &value = prop.second;
            size_t pos = value.find('/');
            bool okay = false;
            if (pos == 0) {
                pos = value.find('/', 1);
                if (pos != value.npos) {
                    std::string revision = m_escape.unescape(value.substr(1, pos - 1));
                    size_t nextpos = value.find('/', pos + 1);
                    if (nextpos != value.npos) {
                        std::string uid = m_escape.unescape(value.substr(pos + 1, nextpos - pos - 1));
                        boost::shared_ptr<Event> &eventptr = m_cache[easid];
                        if (!eventptr) {
                            eventptr = boost::shared_ptr<Event>(new Event);
                        }
                        eventptr->m_easid = easid;
                        eventptr->m_uid = uid;
                        pos = nextpos;
                        while ((nextpos = value.find('/', pos + 1)) != value.npos) {
                            std::string subid = m_escape.unescape(value.substr(pos + 1, nextpos - pos - 1));
                            eventptr->m_subids.insert(subid);
                            pos = nextpos;
                        }
                        okay = true;
                    }
                }
            }
            if (!okay) {
                SE_LOG_DEBUG(this, NULL, "unsupported or corrupt revision entry: %s = %s",
                             easid.c_str(),
                             value.c_str());
            }
        }
    }

    GErrorCXX gerror;
    EASItemsCXX created, updated;
    EASIdsCXX deleted;
    gchar *buffer;
    if (!eas_sync_handler_get_items(getHandler(),
                                    getStartSyncKey().c_str(),
                                    &buffer,
                                    getEasType(),
                                    getFolder().c_str(),
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
        string easid(item->server_id);
        SE_LOG_DEBUG(this, NULL, "new eas item %s", easid.c_str());
        Event &event = setItemData(easid, item->data);
        BOOST_FOREACH(const std::string &subid, event.m_subids) {
            SE_LOG_DEBUG(this, NULL, "new eas item %s = uid %s + rid %s",
                         easid.c_str(), event.m_uid.c_str(), subid.c_str());
            addItem(createLUID(easid, subid), NEW);
        }
    }
    BOOST_FOREACH(EasItemInfo *item, updated) {
        string easid(item->server_id);
        SE_LOG_DEBUG(this, NULL, "updated eas item %s", easid.c_str());
        Event &event = setItemData(easid, item->data);
        BOOST_FOREACH(const std::string &subid, event.m_subids) {
            SE_LOG_DEBUG(this, NULL, "deleted eas item %s = uid %s + rid %s",
                         easid.c_str(), event.m_uid.c_str(), subid.c_str());
            addItem(createLUID(easid, subid), UPDATED);
        }
    }
    BOOST_FOREACH(const char *serverID, deleted) {
        string easid(serverID);
        Event &event = findItem(easid);
        if (event.m_subids.empty()) {
            SE_LOG_DEBUG(this, NULL, "deleted eas item %s empty?!", easid.c_str());
        } else {
            BOOST_FOREACH(const std::string &subid, event.m_subids) {
                SE_LOG_DEBUG(this, NULL, "deleted eas item %s = uid %s + rid %s",
                             easid.c_str(), event.m_uid.c_str(), subid.c_str());
                addItem(createLUID(easid, subid), DELETED);
            }
        }
        m_cache.erase(easid);
    }

    // now also generate full list of all current items:
    // old items + new (added to m_events above) - deleted (removed above)
    BOOST_FOREACH(const EventCache::value_type &entry, m_cache) {
        const std::string &easid = entry.first;
        const boost::shared_ptr<Event> &eventptr = entry.second;
        BOOST_FOREACH(const std::string &subid, eventptr->m_subids) {
            SE_LOG_DEBUG(this, NULL, "existing eas item %s = uid %s + rid %s",
                         easid.c_str(), eventptr->m_uid.c_str(), subid.c_str());
            addItem(createLUID(easid, subid), ANY);
        }
    }

    // update key
    setCurrentSyncKey(buffer);
}

std::string ActiveSyncCalendarSource::endSync(bool success)
{
    m_trackingNode->clear();
    if (success) {
        BOOST_FOREACH(const EventCache::value_type &entry, m_cache) {
            const std::string &easid = entry.first;
            const boost::shared_ptr<Event> &eventptr = entry.second;
            std::stringstream buffer;
            buffer << "//"; // use same format as in MapSyncSource, just in case - was '/' << m_escape.escape(ids.m_revision) << '/';
            buffer << m_escape.escape(eventptr->m_uid) << '/';
            BOOST_FOREACH(const std::string &subid, eventptr->m_subids) {
                buffer << m_escape.escape(subid) << '/';
            }
            m_trackingNode->setProperty(easid, buffer.str());
        }
    } else {
        setCurrentSyncKey("");
    }

    // flush both nodes, just in case; in practice, the properties
    // end up in the same file and only get flushed once
    m_trackingNode->flush();

    return getCurrentSyncKey();
}

std::string ActiveSyncCalendarSource::getDescription(const string &luid)
{
    StringPair ids = MapSyncSource::splitLUID(luid);
    const std::string &easid = ids.first;
    const std::string &subid = ids.second;

    Event &event = findItem(easid);
    if (!event.m_calendar) {
        // Don't load (expensive!) only to provide the description.
        // Returning an empty string will trigger the fallback (logging the ID).
        return "";
    }
    for (icalcomponent *comp = icalcomponent_get_first_component(event.m_calendar, ICAL_VEVENT_COMPONENT);
         comp;
         comp = icalcomponent_get_next_component(event.m_calendar, ICAL_VEVENT_COMPONENT)) {
        if (Event::getSubID(comp) == subid) {
            std::string descr;

            const char *summary = icalcomponent_get_summary(comp);
            if (summary && summary[0]) {
                descr += summary;
            }

            if (true /* is event */) {
                const char *location = icalcomponent_get_location(comp);
                if (location && location[0]) {
                    if (!descr.empty()) {
                        descr += ", ";
                    }
                    descr += location;
                }
            }
            // TODO: other item types
            return descr;
        }
    }
    return "";
}

ActiveSyncCalendarSource::Event &ActiveSyncCalendarSource::findItem(const std::string &easid)
{
    EventCache::iterator it = m_cache.find(easid);
    if (it == m_cache.end()) {
        throwError("event not found");
    }
    return *it->second;
}

ActiveSyncCalendarSource::Event &ActiveSyncCalendarSource::loadItem(const std::string &easid)
{
    Event &event = findItem(easid);
    return loadItem(event);
}

ActiveSyncCalendarSource::Event &ActiveSyncCalendarSource::loadItem(Event &event)
{
    if (!event.m_calendar) {
        std::string item;
        ActiveSyncSource::readItem(event.m_easid, item);
        event.m_calendar.set(icalcomponent_new_from_string((char *)item.c_str()), // hack for old libical
                             "parsing iCalendar 2.0");
    }
    return event;
}

ActiveSyncCalendarSource::Event &ActiveSyncCalendarSource::setItemData(const std::string &easid, const std::string &data)
{
    boost::shared_ptr<Event> &eventptr = m_cache[easid];
    if (eventptr) {
        eventptr->m_uid.clear();
        eventptr->m_subids.clear();
    } else {
        eventptr = boost::shared_ptr<Event>(new Event);
    }

    Event &event = *eventptr;
    event.m_easid = easid;
    event.m_calendar.set(icalcomponent_new_from_string((char *)data.c_str()), // hack for old libical
                         "parsing iCalendar 2.0");
    for (icalcomponent *comp = icalcomponent_get_first_component(event.m_calendar, ICAL_VEVENT_COMPONENT);
         comp;
         comp = icalcomponent_get_next_component(event.m_calendar, ICAL_VEVENT_COMPONENT)) {
        if (event.m_uid.empty()) {
            event.m_uid = Event::getUID(comp);
        }
        std::string subid = Event::getSubID(comp);
        event.m_subids.insert(subid);
    }
    return event;
}

std::string ActiveSyncCalendarSource::Event::icalTime2Str(const icaltimetype &tt)
{
    static const struct icaltimetype null = { 0 };
    if (!memcmp(&tt, &null, sizeof(null))) {
        return "";
    } else {
        eptr<char> timestr(ical_strdup(icaltime_as_ical_string(tt)));
        if (!timestr) {
            SE_THROW("cannot convert to time string");
        }
        return timestr.get();
    }
}

std::string ActiveSyncCalendarSource::Event::getSubID(icalcomponent *comp)
{
    struct icaltimetype rid = icalcomponent_get_recurrenceid(comp);
    return icalTime2Str(rid);
}

std::string ActiveSyncCalendarSource::Event::getUID(icalcomponent *comp)
{
    std::string uid;
    icalproperty *prop = icalcomponent_get_first_property(comp, ICAL_UID_PROPERTY);
    if (prop) {
        uid = icalproperty_get_uid(prop);
    }
    return uid;
}

void ActiveSyncCalendarSource::Event::setUID(icalcomponent *comp, const std::string &uid)
{
    icalproperty *prop = icalcomponent_get_first_property(comp, ICAL_UID_PROPERTY);
    if (prop) {
        icalproperty_set_uid(prop, uid.c_str());
    } else {
        icalcomponent_add_property(comp, icalproperty_new_uid(uid.c_str()));
    }
}

ActiveSyncCalendarSource::EventCache::iterator ActiveSyncCalendarSource::EventCache::findByUID(const std::string &uid)
{
    for (iterator it = begin();
         it != end();
         ++it) {
        if (it->second->m_uid == uid) {
            return it;
        }
    }
    return end();
}

SyncSourceRaw::InsertItemResult ActiveSyncCalendarSource::insertItem(const std::string &luid, const std::string &item)
{
    StringPair ids = splitLUID(luid);
    const std::string &callerEasID = ids.first;
    std::string easid = ids.first;
    const std::string &callerSubID = ids.second;

    // parse new event
    boost::shared_ptr<Event> newEvent(new Event);
    newEvent->m_calendar.set(icalcomponent_new_from_string((char *)item.c_str()), // hack for old libical
                             "parsing iCalendar 2.0");
    icalcomponent *firstcomp = NULL;
    for (icalcomponent *comp = firstcomp = icalcomponent_get_first_component(newEvent->m_calendar, ICAL_VEVENT_COMPONENT);
         comp;
         comp = icalcomponent_get_next_component(newEvent->m_calendar, ICAL_VEVENT_COMPONENT)) {
        std::string subid = Event::getSubID(comp);
        EventCache::iterator it;
        if (!luid.empty() &&
            (it = m_cache.find(luid)) != m_cache.end()) {
            // Additional sanity check: ensure that the expected UID is set.
            // Necessary if the peer we synchronize with (aka the local
            // data storage) doesn't support foreign UIDs. Maemo 5 calendar
            // backend is one example.
            Event::setUID(comp, it->second->m_uid);
            newEvent->m_uid = it->second->m_uid;
        } else {
            newEvent->m_uid = Event::getUID(comp);
            if (newEvent->m_uid.empty()) {
                // create new UID
                newEvent->m_uid = UUID();
                Event::setUID(comp, newEvent->m_uid);
            }
        }
        newEvent->m_subids.insert(subid);
    }
    if (newEvent->m_subids.size() != 1) {
        SE_THROW("new CalDAV item did not contain exactly one VEVENT");
    }
    std::string subid = *newEvent->m_subids.begin();

    // Determine whether we already know the merged item even though
    // our caller didn't.
    std::string knownSubID = callerSubID;
    if (easid.empty()) {
        EventCache::iterator it = m_cache.findByUID(newEvent->m_uid);
        if (it != m_cache.end()) {
            easid = it->first;
            knownSubID = subid;
        }
    }

    bool merged = false;
    if (easid.empty()) {
        // New VEVENT; should not be part of an existing merged item
        // ("meeting series").
        InsertItemResult res = ActiveSyncSource::insertItem("", item);
        easid = res.m_luid;

        EventCache::iterator it = m_cache.find(res.m_luid);
        if (it != m_cache.end()) {
            // merge into existing Event
            Event &event = loadItem(*it->second);
            if (event.m_subids.find(subid) != event.m_subids.end()) {
                // was already in that item but caller didn't seem to know
                merged = true;
            } else {
                // add to merged item
                event.m_subids.insert(subid);
            }
            icalcomponent_merge_component(event.m_calendar,
                                          newEvent->m_calendar.release()); // function destroys merged calendar
        } else {
            // add to cache without further changes
            newEvent->m_easid = res.m_luid;
            m_cache[newEvent->m_easid] = newEvent;
        }
    } else {
        if (subid != knownSubID) {
            SE_THROW(StringPrintf("update for eas item %s rid %s has wrong rid %s",
                                  easid.c_str(),
                                  knownSubID.c_str(),
                                  subid.c_str()));
        }
        Event &event = findItem(easid);
        if (event.m_subids.size() == 1 &&
            *event.m_subids.begin() == subid) {
            // special case: no need to load old data, replace it outright
            event.m_calendar = newEvent->m_calendar;
        } else {
            // populate event
            loadItem(event);

            // update cache: find old VEVENT and remove it before adding new one
            icalcomponent *removeme = NULL;
            for (icalcomponent *comp = icalcomponent_get_first_component(event.m_calendar, ICAL_VEVENT_COMPONENT);
                 comp;
                 comp = icalcomponent_get_next_component(event.m_calendar, ICAL_VEVENT_COMPONENT)) {
                if (Event::getSubID(comp) == subid) {
                    removeme = comp;
                    break;
                }
            }
            if (easid != callerEasID) {
                // caller didn't know final UID: if found, then tell him that
                // we merged the item for him, if not, then don't complain about
                // it not being found (like we do when the item should exist
                // but doesn't)
                if (removeme) {
                    merged = true;
                    icalcomponent_remove_component(event.m_calendar, removeme);
                } else {
                    event.m_subids.insert(subid);
                }
            } else {
                if (removeme) {
                    // this is what we expect when the caller mentions the ActiveSync ID
                    icalcomponent_remove_component(event.m_calendar, removeme);
                } else {
                    // caller confused?!
                    SE_THROW("event not found");
                }
            }

            icalcomponent_merge_component(event.m_calendar,
                                          newEvent->m_calendar.release()); // function destroys merged calendar
        }

        eptr<char> icalstr(ical_strdup(icalcomponent_as_ical_string(event.m_calendar)));
        std::string data = icalstr.get();

        // TODO: avoid updating item on server immediately?
        InsertItemResult res = ActiveSyncSource::insertItem(event.m_easid, data);
        if (res.m_merged ||
            res.m_luid != event.m_easid) {
            // should not merge with anything, if so, our cache was invalid
            SE_THROW("CalDAV item not updated as expected");
        }
    }

    return SyncSourceRaw::InsertItemResult(createLUID(easid, subid),
                                           "", merged);
}

void ActiveSyncCalendarSource::readItem(const std::string &luid, std::string &item)
{
    StringPair ids = splitLUID(luid);
    const std::string &easid = ids.first;
    const std::string &subid = ids.second;

    Event &event = loadItem(easid);
    if (event.m_subids.size() == 1) {
        // simple case: convert existing VCALENDAR
        if (*event.m_subids.begin() == subid) {
            eptr<char> icalstr(ical_strdup(icalcomponent_as_ical_string(event.m_calendar)));
            item = icalstr.get();
        } else {
            SE_THROW("event not found");
        }
    } else {
        // complex case: create VCALENDAR with just the VTIMEZONE definition(s)
        // and the one event, then convert that
        eptr<icalcomponent> calendar(icalcomponent_new(ICAL_VCALENDAR_COMPONENT), "VCALENDAR");
        for (icalcomponent *tz = icalcomponent_get_first_component(event.m_calendar, ICAL_VTIMEZONE_COMPONENT);
             tz;
             tz = icalcomponent_get_next_component(event.m_calendar, ICAL_VTIMEZONE_COMPONENT)) {
            eptr<icalcomponent> clone(icalcomponent_new_clone(tz), "VTIMEZONE");
            icalcomponent_add_component(calendar, clone.release());
        }
        bool found = false;
        for (icalcomponent *comp = icalcomponent_get_first_component(event.m_calendar, ICAL_VEVENT_COMPONENT);
             comp;
             comp = icalcomponent_get_next_component(event.m_calendar, ICAL_VEVENT_COMPONENT)) {
            if (Event::getSubID(comp) == subid) {
                eptr<icalcomponent> clone(icalcomponent_new_clone(comp), "VEVENT");
                icalcomponent_add_component(calendar, clone.release());
                found = true;
                break;
            }
        }
        if (!found) {
            SE_THROW("event not found");
        }
        eptr<char> icalstr(ical_strdup(icalcomponent_as_ical_string(calendar)));
        item = icalstr.get();
    }
}

void ActiveSyncCalendarSource::deleteItem(const string &luid)
{
    StringPair ids = splitLUID(luid);
    const std::string &easid = ids.first;
    const std::string &subid = ids.second;

    // find item in cache first, load only if it is not going to be
    // removed entirely
    Event &event = findItem(easid);

    if (event.m_subids.size() == 1) {
        // remove entire merged item, nothing will be left after removal
        if (*event.m_subids.begin() != subid) {
            SE_THROW("event not found");
        } else {
            event.m_subids.clear();
            event.m_calendar = NULL;
            ActiveSyncSource::deleteItem(ids.first);
        }
        m_cache.erase(easid);
    } else {
        loadItem(event);
        bool found = false;
        // bool parentRemoved = false;
        for (icalcomponent *comp = icalcomponent_get_first_component(event.m_calendar, ICAL_VEVENT_COMPONENT);
             comp;
             comp = icalcomponent_get_next_component(event.m_calendar, ICAL_VEVENT_COMPONENT)) {
            if (Event::getSubID(comp) == subid) {
                icalcomponent_remove_component(event.m_calendar, comp);
                icalcomponent_free(comp);
                found = true;
                // if (subid.empty()) {
                // parentRemoved = true;
                // }
            }
        }
        if (!found) {
            SE_THROW("event not found");
        }
        event.m_subids.erase(subid);
        // TODO: avoid updating the item immediately
        eptr<char> icalstr(ical_strdup(icalcomponent_as_ical_string(event.m_calendar)));
        InsertItemResult res = ActiveSyncSource::insertItem(easid, icalstr.get());
        if (res.m_merged ||
            res.m_luid != easid) {
            SE_THROW("unexpected result of removing sub event");
        }
    }
}

std::string ActiveSyncCalendarSource::createLUID(const std::string &uid, const std::string &subid)
{
    std::string luid = m_escape.escape(uid);
    if (!subid.empty()) {
        luid += '/';
        luid += m_escape.escape(subid);
    }
    return luid;
}

std::pair<std::string, std::string> ActiveSyncCalendarSource::splitLUID(const std::string &luid)
{
    size_t index = luid.find('/');
    if (index != luid.npos) {
        return make_pair(m_escape.unescape(luid.substr(0, index)),
                         m_escape.unescape(luid.substr(index + 1)));
    } else {
        return make_pair(m_escape.unescape(luid), "");
    }
}

StringEscape ActiveSyncCalendarSource::m_escape('%', "/");


SE_END_CXX

#endif // ENABLE_ACTIVESYNC
