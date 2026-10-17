#include "torrentscount.hpp"

#include "../resolve.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"
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
    auto resolved_session = ResolveSession(m_sessions, req.session_id);

    TorrentsCountRes res{};
    res.total = resolved_session.session->Count();

    for (const auto& [ _, t ] : resolved_session.session->Torrents())
    {
        const auto& ts = t.status;

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

        res.trackers[ts.current_tracker]++;

        if (t.data)
        {
            if (t.data->category.has_value() && !t.data->category->empty())
            {
                res.categories[t.data->category.value()]++;
            }

            for (const auto& tag : t.data->tags)
            {
                if (tag.empty()) continue;
                res.tags[tag]++;
            }
        }
    }

    return cb->Ok(res);
}
