#include "torrentstrackersadd.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsTrackersAdd;
using porla::Rpc::Methods::Torrents::TorrentsTrackersAddReq;
using porla::Rpc::Methods::Torrents::TorrentsTrackersAddRes;

TorrentsTrackersAdd::TorrentsTrackersAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsTrackersAdd::Execute(const TorrentsTrackersAddReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    lt::announce_entry ae;
    ae.url = req.url;

    if (req.tier.has_value())
    {
        ae.tier = req.tier.value();
    }

    resolved->torrent->status.handle.add_tracker(ae);

    cb->Ok(TorrentsTrackersAddRes{});
}
