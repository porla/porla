#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <boost/asio/io_context.hpp>

#include "../src/events.hpp"
#include "../src/sessions/sessionevent.hpp"
#include "../src/sessions/sessions.hpp"
#include "../src/sessions/torrentevent.hpp"

using porla::Event;
using porla::Events;

namespace
{
    struct TestEvent : Event
    {
        using Event::Event;

        int value = 0;
    };

    class EventsTest : public ::testing::Test
    {
    protected:
        // runs everything queued so far and returns the number of handlers that ran
        std::size_t Drain()
        {
            const auto ran = io.run();
            io.restart();
            return ran;
        }

        boost::asio::io_context io;
        Events                  events{ io };
    };
}

TEST_F(EventsTest, Publish_WithSubscriber_DeliversWhenIoRuns)
{
    int calls = 0;
    auto c = events.On("test.event", [&](const Event&) { calls++; });

    events.Publish(Event("test.event"));

    EXPECT_EQ(calls, 0) << "delivery must not happen inside Publish";

    Drain();

    EXPECT_EQ(calls, 1);
}

TEST_F(EventsTest, Publish_WithoutSubscribers_QueuesNothing)
{
    events.Publish(Event("test.event"));

    EXPECT_EQ(Drain(), 0);
}

TEST_F(EventsTest, Publish_WithSubscriberForOtherName_QueuesNothing)
{
    int calls = 0;
    auto c = events.On("test.other", [&](const Event&) { calls++; });

    events.Publish(Event("test.event"));

    EXPECT_EQ(Drain(), 0);
    EXPECT_EQ(calls, 0);
}

TEST_F(EventsTest, On_ReceivesOnlyItsOwnName)
{
    std::vector<std::string> a, b;
    auto ca = events.On("test.a", [&](const Event& e) { a.push_back(e.name); });
    auto cb = events.On("test.b", [&](const Event& e) { b.push_back(e.name); });

    events.Publish(Event("test.a"));
    events.Publish(Event("test.b"));
    events.Publish(Event("test.a"));
    Drain();

    EXPECT_EQ(a, (std::vector<std::string>{ "test.a", "test.a" }));
    EXPECT_EQ(b, (std::vector<std::string>{ "test.b" }));
}

TEST_F(EventsTest, Publish_DeliversInPublishOrder)
{
    std::vector<int> seen;
    auto c1 = events.On("test.one", [&](const Event& e) { seen.push_back(e.data["n"].get<int>()); });
    auto c2 = events.On("test.two", [&](const Event& e) { seen.push_back(e.data["n"].get<int>()); });

    for (int i = 0; i < 10; i++)
    {
        events.Publish(Event(i % 2 == 0 ? "test.one" : "test.two", { { "n", i } }));
    }

    Drain();

    EXPECT_EQ(seen, (std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }));
}

TEST_F(EventsTest, Publish_AssignsUniqueIncreasingIds)
{
    std::vector<std::uint64_t> ids;
    auto c = events.On("test.event", [&](const Event& e) { ids.push_back(e.id); });

    for (int i = 0; i < 5; i++)
    {
        events.Publish(Event("test.event"));
    }

    Drain();

    ASSERT_EQ(ids.size(), 5);
    EXPECT_GT(ids.front(), 0) << "0 is reserved for events that were never published";

    for (std::size_t i = 1; i < ids.size(); i++)
    {
        EXPECT_GT(ids[i], ids[i - 1]);
    }
}

TEST_F(EventsTest, Event_NotPublished_HasIdZero)
{
    EXPECT_EQ(Event("test.event").id, 0);
}

TEST_F(EventsTest, Event_WithoutData_HasEmptyObject)
{
    const Event e("test.event");

    EXPECT_TRUE(e.data.is_object());
    EXPECT_TRUE(e.data.empty());
}

TEST_F(EventsTest, Publish_DerivedEvent_ArrivesAsItsOwnType)
{
    int value = 0;
    auto c = events.On("test.event", [&](const Event& e)
    {
        const auto* te = dynamic_cast<const TestEvent*>(&e);
        ASSERT_NE(te, nullptr);
        value = te->value;
    });

    TestEvent event("test.event");
    event.value = 42;

    events.Publish(std::move(event));
    Drain();

    EXPECT_EQ(value, 42);
}

TEST_F(EventsTest, Publish_AllSubscribersGetTheSameEvent)
{
    const Event* first  = nullptr;
    const Event* second = nullptr;

    auto c1 = events.On("test.event", [&](const Event& e) { first = &e; });
    auto c2 = events.On("test.event", [&](const Event& e) { second = &e; });

    events.Publish(Event("test.event"));
    Drain();

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second) << "the Lua table cache relies on one event object per delivery";
}

TEST_F(EventsTest, HasSubscribers_FollowsConnectAndDisconnect)
{
    EXPECT_FALSE(events.HasSubscribers("test.event"));

    auto c = events.On("test.event", [](const Event&) {});
    EXPECT_TRUE(events.HasSubscribers("test.event"));
    EXPECT_FALSE(events.HasSubscribers("test.other"));

    c.disconnect();
    EXPECT_FALSE(events.HasSubscribers("test.event"));
}

TEST_F(EventsTest, Disconnect_BeforeDelivery_IsNotDelivered)
{
    int calls = 0;
    auto c = events.On("test.event", [&](const Event&) { calls++; });

    events.Publish(Event("test.event"));
    c.disconnect();
    Drain();

    EXPECT_EQ(calls, 0);
}

TEST_F(EventsTest, Subscribe_AfterPublish_StillReceivesQueuedEvent)
{
    int early = 0;
    int late  = 0;

    auto c1 = events.On("test.event", [&](const Event&) { early++; });
    events.Publish(Event("test.event"));
    auto c2 = events.On("test.event", [&](const Event&) { late++; });
    Drain();

    EXPECT_EQ(early, 1);
    EXPECT_EQ(late, 1) << "subscribers are looked up at delivery time";
}

TEST_F(EventsTest, Subscribe_AfterUnsubscribedPublish_DoesNotReceiveIt)
{
    int calls = 0;

    events.Publish(Event("test.event"));
    auto c = events.On("test.event", [&](const Event&) { calls++; });
    Drain();

    EXPECT_EQ(calls, 0) << "events nobody listened to at publish time are dropped";
}

TEST_F(EventsTest, Disconnect_SelfDuringDelivery_StopsLaterEvents)
{
    int calls = 0;
    boost::signals2::connection c;
    c = events.On("test.event", [&](const Event&)
    {
        calls++;
        c.disconnect();
    });

    events.Publish(Event("test.event"));
    events.Publish(Event("test.event"));
    Drain();

    EXPECT_EQ(calls, 1);
    EXPECT_FALSE(events.HasSubscribers("test.event"));
}

TEST_F(EventsTest, Disconnect_OtherDuringDelivery_SkipsIt)
{
    int a = 0;
    int b = 0;
    boost::signals2::connection cb;

    auto ca = events.On("test.event", [&](const Event&) { a++; cb.disconnect(); });
    cb      = events.On("test.event", [&](const Event&) { b++; });

    events.Publish(Event("test.event"));
    Drain();

    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 0) << "a slot disconnected earlier in the same delivery is not called";
}

TEST_F(EventsTest, Publish_FromHandler_IsDeliveredAfterCurrentEvent)
{
    std::vector<std::string> seen;

    auto c1 = events.On("test.first", [&](const Event& e)
    {
        seen.push_back(e.name + ":start");
        events.Publish(Event("test.second"));
        seen.push_back(e.name + ":end");
    });

    auto c2 = events.On("test.second", [&](const Event& e) { seen.push_back(e.name); });

    events.Publish(Event("test.first"));
    Drain();

    EXPECT_EQ(seen, (std::vector<std::string>{ "test.first:start", "test.first:end", "test.second" }));
}

TEST_F(EventsTest, Subscribe_FromHandler_ToNewName_Works)
{
    int calls = 0;
    boost::signals2::connection inner;

    auto outer = events.On("test.first", [&](const Event&)
    {
        inner = events.On("test.second", [&](const Event&) { calls++; });
        events.Publish(Event("test.second"));
    });

    events.Publish(Event("test.first"));
    Drain();

    EXPECT_EQ(calls, 1);
}

TEST(SessionEvents, TorrentEvent_IsASessionEvent)
{
    porla::TorrentEvent te("torrent.added");
    te.session_id = 3;

    const Event& e = te;

    const auto* se = dynamic_cast<const porla::SessionEvent*>(&e);
    ASSERT_NE(se, nullptr);
    EXPECT_EQ(se->session_id, 3);
    EXPECT_NE(dynamic_cast<const porla::TorrentEvent*>(&e), nullptr);
}

TEST(SessionEvents, SessionEvent_IsNotATorrentEvent)
{
    const porla::SessionEvent se("session.added");
    const Event& e = se;

    EXPECT_NE(dynamic_cast<const porla::SessionEvent*>(&e), nullptr);
    EXPECT_EQ(dynamic_cast<const porla::TorrentEvent*>(&e), nullptr);
}

TEST(SessionEvents, Defaults_AreEmpty)
{
    const porla::TorrentEvent te("torrent.removed");

    EXPECT_EQ(te.session_id, -1);
    EXPECT_EQ(te.session.lock(), nullptr);
    EXPECT_FALSE(te.torrent_handle.is_valid());
    EXPECT_FALSE(te.info_hash.has_v1() || te.info_hash.has_v2());
}
