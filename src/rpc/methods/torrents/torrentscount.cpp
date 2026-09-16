#include "torrentscount.hpp"

#include "../../../sessions.hpp"
#include "../../../torrentclientdata.hpp"

using porla::Rpc::Methods::Torrents::TorrentsCount;
using porla::Rpc::Methods::Torrents::TorrentsCountReq;
using porla::Rpc::Methods::Torrents::TorrentsCountRes;

TorrentsCount::TorrentsCount(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsCount::Execute(const TorrentsCountReq& req, ResponseWriterHandle cb)
{
    const auto& session_state = m_sessions.Get(req.session_id);

    if (session_state == nullptr)
    {
        return cb->Error(-1, "Session not found");
    }

    TorrentsCountRes res{};
    res.total = session_state->torrents.size();

    for (const auto& [ _, pair ] : session_state->torrents)
    {
        const auto& [ th, ts ] = pair;
        const auto  userdata   = th.userdata().get<TorrentClientData>();

        if ((ts.state == lt::torrent_status::state_t::downloading
            || ts.state == lt::torrent_status::state_t::downloading_metadata)
            && !(ts.flags & lt::torrent_flags::paused))
        {
            res.downloading++;
        }

        if ((ts.state == lt::torrent_status::state_t::downloading
            || ts.state == lt::torrent_status::state_t::downloading_metadata)
            && (ts.flags & lt::torrent_flags::auto_managed)
            && (ts.flags & lt::torrent_flags::paused))
        {
            res.downloading_queued++;
        }

        if (ts.errc)
        {
            res.error++;
        }

        if ((ts.state == lt::torrent_status::state_t::finished
            || ts.state == lt::torrent_status::state_t::seeding)
            && !(ts.flags & lt::torrent_flags::auto_managed)
            && (ts.flags & lt::torrent_flags::paused))
        {
            res.finished++;
        }

        if ((ts.state == lt::torrent_status::state_t::downloading
            || ts.state == lt::torrent_status::state_t::downloading_metadata)
            && !(ts.flags & lt::torrent_flags::auto_managed)
            && (ts.flags & lt::torrent_flags::paused))
        {
            res.paused++;
        }

        if ((ts.state == lt::torrent_status::state_t::finished
            || ts.state == lt::torrent_status::state_t::seeding)
            && !(ts.flags & lt::torrent_flags::paused))
        {
            res.seeding++;
        }

        if ((ts.state == lt::torrent_status::state_t::finished
            || ts.state == lt::torrent_status::state_t::seeding)
            && (ts.flags & lt::torrent_flags::auto_managed)
            && (ts.flags & lt::torrent_flags::paused))
        {
            res.seeding_queued++;
        }

        if (userdata->category.has_value())
        {
            if (userdata->category.value().empty()) continue;
            res.categories[userdata->category.value()]++;
        }

        res.trackers[ts.current_tracker]++;

        for (const auto& tag : userdata->tags)
        {
            if (tag.empty()) continue;
            res.tags[tag]++;
        }
    }

    return cb->Ok(res);
}
