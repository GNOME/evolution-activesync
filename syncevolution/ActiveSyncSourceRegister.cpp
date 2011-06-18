/*
 * Copyright (C) 2008-2009 Patrick Ohly <patrick.ohly@gmx.de>
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

#include "ActiveSyncSource.h"

#ifdef ENABLE_UNIT_TESTS
# include <cppunit/extensions/TestFactoryRegistry.h>
# include <cppunit/extensions/HelperMacros.h>
#endif

#include <syncevo/declarations.h>
SE_BEGIN_CXX

static SyncSource *createSource(const SyncSourceParams &params)
{
    SourceType sourceType = SyncSource::getSourceType(params.m_nodes);
    bool isMe;

    isMe = sourceType.m_backend == "ActiveSync Address Book";
    if (isMe) {
        return
#ifdef ENABLE_ACTIVESYNC
            new ActiveSyncContactSource(params)
#else
            RegisterSyncSource::InactiveSource
#endif
            ;
    }

    isMe = sourceType.m_backend == "ActiveSync Events";
    if (isMe) {
        return
#ifdef ENABLE_ACTIVESYNC
            new ActiveSyncCalendarSource(params, EAS_ITEM_CALENDAR)
#else
            RegisterSyncSource::InactiveSource
#endif
            ;
    }

    isMe = sourceType.m_backend == "ActiveSync Todos";
    if (isMe) {
        return
#ifdef ENABLE_ACTIVESYNC
            new ActiveSyncCalendarSource(params, EAS_ITEM_TODO)
#else
            RegisterSyncSource::InactiveSource
#endif
            ;
    }

    isMe = sourceType.m_backend == "ActiveSync Memos";
    if (isMe) {
        return
#ifdef ENABLE_ACTIVESYNC
            new ActiveSyncCalendarSource(params, EAS_ITEM_JOURNAL)
#else
            RegisterSyncSource::InactiveSource
#endif
            ;
    }

    return NULL;
}

static RegisterSyncSource registerMe("ActiveSync",
#ifdef ENABLE_ACTIVESYNC
                                     true,
#else
                                     false,
#endif
                                     createSource,
                                     "ActiveSync Address Book = eas-contacts\n"
                                     "ActiveSync Events = eas-events\n"
                                     "ActiveSync Todos = eas-todos\n"
                                     "ActiveSync Memos = eas-memos",
                                     Values() +
                                     (Aliases("ActiveSync Address Book") + "eas-contacts") +
                                     (Aliases("ActiveSync Events") + "eas-events") +
                                     (Aliases("ActiveSync Todos") + "eas-todos") +
                                     (Aliases("ActiveSync Memos") + "eas-memos"));

#ifdef ENABLE_ACTIVESYNC
#ifdef ENABLE_UNIT_TESTS

class ActiveSyncsTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ActiveSyncsTest);
    CPPUNIT_TEST(testInstantiate);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testInstantiate() {
        boost::shared_ptr<SyncSource> source;
        source.reset(SyncSource::createTestingSource("contacts", "ActiveSync Address Book", true));
        source.reset(SyncSource::createTestingSource("events", "ActiveSync Events", true));
        source.reset(SyncSource::createTestingSource("todos", "ActiveSync Todos", true));
        source.reset(SyncSource::createTestingSource("memos", "ActiveSync Memos", true));
    }
};

SYNCEVOLUTION_TEST_SUITE_REGISTRATION(ActiveSyncsTest);

#endif // ENABLE_UNIT_TESTS

#ifdef ENABLE_INTEGRATION_TESTS
namespace {
#if 0
}
#endif

static class ActiveSyncContactTest : public RegisterSyncSourceTest {
public:
    ActiveSyncContactTest() :
        RegisterSyncSourceTest("eas_contact", // name of test => Client::Source::eas_contact"
                               "eds_contact"  // name of test cases: inherit from EDS, override below
                               ) {}

    virtual void updateConfig(ClientTestConfig &config) const
    {
        // override default eds_contact test config
        config.type = "eas-contacts";
        // TODO: provide comprehensive set of vCard 3.0 contacts as they are understood by the ActiveSync library
        // config.testcases = "testcases/eas_contact.vcf";
    }
} ActiveSyncContactTest;

static class ActiveSyncEventTest : public RegisterSyncSourceTest {
public:
    ActiveSyncEventTest() :
        RegisterSyncSourceTest("eas_event", "eds_event")
    {}

    virtual void updateConfig(ClientTestConfig &config) const
    {
    }
} ActiveSyncEventTest;

static class ActiveSyncTodoTest : public RegisterSyncSourceTest {
public:
    ActiveSyncTodoTest() :
        RegisterSyncSourceTest("eas_todo", "eds_todo")
    {}

    virtual void updateConfig(ClientTestConfig &config) const
    {
    }
} ActiveSyncTodoTest;

static class ActiveSyncMemoTest : public RegisterSyncSourceTest {
public:
    ActiveSyncMemoTest() :
        RegisterSyncSourceTest("eas_memo", "eds_memo")
    {}

    virtual void updateConfig(ClientTestConfig &config) const
    {
    }
} ActiveSyncMemoTest;

}
#endif // ENABLE_INTEGRATION_TESTS

#endif // ENABLE_ACTIVESYNC

SE_END_CXX
