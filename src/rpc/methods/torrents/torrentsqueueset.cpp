#include "torrentsqueueset.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsQueueSet;
using porla::Rpc::Methods::Torrents::TorrentsQueueSetReq;
using porla::Rpc::Methods::Torrents::TorrentsQueueSetRes;

TorrentsQueueSet::TorrentsQueueSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsQueueSet::Execute(const TorrentsQueueSetReq &req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->torrent->status.handle.queue_position_set(req.queue_position);

    return cb->Ok(TorrentsQueueSetRes{});
}
