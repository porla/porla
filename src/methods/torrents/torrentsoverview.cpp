#include "torrentsoverview.hpp"

#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"

using porla::Methods::Torrents::TorrentsOverview;
using porla::Methods::Torrents::TorrentsOverviewSession;
using porla::Sessions;

static TorrentsOverviewSession GetSessionOverview(const std::shared_ptr<Sessions::SessionState>& state)
{
    TorrentsOverviewSession sess{
        .session_id     = state->id,
        .session_name   = state->name,
        .torrents_total = state->torrents.size()
    };

    std::map<std::uint64_t, std::uint64_t> torrents_per_flags;

    for (const auto& [ _, torrent ] : state->torrents)
    {
        const auto& handle = std::get<lt::torrent_handle>(torrent);
        const auto flags   = static_cast<std::uint64_t>(handle.flags());
        const auto& ts     = handle.status();
        const auto data    = handle.userdata().get<porla::TorrentClientData>();

        if (ts.errc)
        {
            sess.torrents_errors++;
        }

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

        if (!torrents_per_flags.contains(flags))
        {
            torrents_per_flags.insert({ flags, 0 });
        }

        torrents_per_flags.at(flags)++;

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

    for (const auto &[flags,count] : torrents_per_flags)
    {
        std::vector<std::uint64_t> v;
        v.emplace_back(flags);
        v.emplace_back(count);

        sess.torrents_per_flags.emplace_back(v);
    }

    return sess;
}

TorrentsOverview::TorrentsOverview(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsOverview::Invoke(const TorrentsOverviewReq& req, WriteCb<TorrentsOverviewRes> cb)
{
    std::optional<int> filter_session_id;

    if (req.filters.has_value()
        && req.filters->contains("session_id")
        && req.filters->at("session_id").is_number_integer())
    {
        filter_session_id = req.filters->at("session_id").get<int>();
    }

    if (filter_session_id.has_value())
    {
        const auto& state = m_sessions.Get(filter_session_id.value());

        if (state == nullptr)
        {
            return cb.Error(-1, "Session not found");
        }

        return cb.Ok(TorrentsOverviewRes{
            .sessions = { GetSessionOverview(state) }
        });
    }

    TorrentsOverviewRes res = {};

    for (const auto& [ _, state ] : m_sessions.All())
    {
        res.sessions.emplace_back(GetSessionOverview(state));
    }

    cb.Ok(res);
}
