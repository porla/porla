#pragma once

#include <libtorrent/session.hpp>

#include "job.hpp"
#include "../session.hpp"
#include "../../events.hpp"

namespace porla::Jobs
{
    class PostDhtStats : public Job
    {
    public:
        explicit PostDhtStats(Events& events)
            : m_events(events)
        {
        }

        std::string_view Name() const override { return "post-dht-stats"; }

        Result Run(Session& session) override
        {
            if (m_events.HasSubscribers("session.dht_stats"))
            {
                session.Libtorrent().post_dht_stats();
            }

            return Done();
        }

    private:
        Events& m_events;
    };

    class PostSessionStats : public Job
    {
    public:
        explicit PostSessionStats(Events& events)
            : m_events(events)
        {
        }

        std::string_view Name() const override { return "post-session-stats"; }

        Result Run(Session& session) override
        {
            if (m_events.HasSubscribers("session.stats"))
            {
                session.Libtorrent().post_session_stats();
            }

            return Done();
        }

    private:
        Events& m_events;
    };

    class PostTorrentUpdates : public Job
    {
    public:
        std::string_view Name() const override { return "post-torrent-updates"; }

        Result Run(Session& session) override
        {
            session.Libtorrent().post_torrent_updates();
            return Done();
        }
    };
}
