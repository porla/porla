#include "torrentscount.hpp"

#include "../../sessions.hpp"

using porla::Methods::Torrents::TorrentsCount;
using porla::Methods::Torrents::TorrentsCountReq;
using porla::Methods::Torrents::TorrentsCountRes;

TorrentsCount::TorrentsCount(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsCount::Invoke(const TorrentsCountReq& req, WriteCb<TorrentsCountRes> cb)
{
    const auto& session_state = m_sessions.Get(req.session_id);

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    TorrentsCountRes res{};
    res.total = session_state->torrents.size();

    for (const auto& [ _, pair ] : session_state->torrents)
    {
        const auto& [ th, ts ] = pair;

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
    }

    return cb.Ok(res);
}
