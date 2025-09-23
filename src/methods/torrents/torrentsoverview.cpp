#include "torrentsoverview.hpp"

#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"

using porla::Methods::Torrents::TorrentsOverview;

TorrentsOverview::TorrentsOverview(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsOverview::Invoke(const TorrentsOverviewReq& req, WriteCb<TorrentsOverviewRes> cb)
{
    TorrentsOverviewRes res = {};

    for (const auto& [ session, state ] : m_sessions.All())
    {
        TorrentsOverviewSession sess{
            .torrents_total = state->torrents.size()
        };

        for (const auto& [ _, torrent ] : state->torrents)
        {
            const auto& handle = std::get<lt::torrent_handle>(torrent);
            const auto data = handle.userdata().get<TorrentClientData>();

            if (data->category.has_value())
            {
                if (!sess.torrents_per_category.contains(data->category.value()))
                {
                    sess.torrents_per_category.insert({ data->category.value(), 0 });
                }

                sess.torrents_per_category.at(data->category.value())++;
            }

            if (!data->tags.empty())
            {
                for (const auto& tag : data->tags)
                {
                    if (!sess.torrents_per_tag.contains(tag))
                    {
                        sess.torrents_per_tag.insert({ tag, 0 });
                    }

                    sess.torrents_per_tag.at(tag)++;
                }
            }

            const auto& ts = handle.status();

            std::string state = "unknown";
            if (ts.state == lt::torrent_status::checking_files)            state = "checking_files";
            else if (ts.state == lt::torrent_status::downloading_metadata) state = "downloading_metadata";
            else if (ts.state == lt::torrent_status::downloading)          state = "downloading";
            else if (ts.state == lt::torrent_status::finished)             state = "finished";
            else if (ts.state == lt::torrent_status::seeding)              state = "seeding";
            else if (ts.state == lt::torrent_status::checking_resume_data) state = "checking_resume_data";

            if (!sess.torrents_per_state.contains(state))
            {
                sess.torrents_per_state.insert({ state, 0 });
            }

            sess.torrents_per_state.at(state)++;

            for (const auto& entry : handle.trackers())
            {
                if (!sess.torrents_per_tracker.contains(entry.url))
                {
                    sess.torrents_per_tracker.insert({ entry.url, 0 });
                }

                sess.torrents_per_tracker.at(entry.url)++;
            }
        }

        res.sessions.insert({ session, sess });
    }

    cb.Ok(res);
}
