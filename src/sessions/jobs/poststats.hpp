#pragma once

#include <libtorrent/session.hpp>

#include "job.hpp"
#include "../session.hpp"

namespace porla::Jobs
{
    class PostDhtStats : public Job
    {
    public:
        std::string_view Name() const override { return "post-dht-stats"; }

        Result Run(Session& session) override
        {
            session.Libtorrent().post_dht_stats();
            return Done();
        }
    };

    class PostSessionStats : public Job
    {
    public:
        std::string_view Name() const override { return "post-session-stats"; }

        Result Run(Session& session) override
        {
            session.Libtorrent().post_session_stats();
            return Done();
        }
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
