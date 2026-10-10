#include "torrentstrackerslist.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsTrackersList;
using porla::Rpc::Methods::Torrents::TorrentsTrackersListReq;
using porla::Rpc::Methods::Torrents::TorrentsTrackersListRes;

TorrentsTrackersList::TorrentsTrackersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsTrackersList::Execute(const TorrentsTrackersListReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    cb->Ok(TorrentsTrackersListRes{
        .trackers = resolved->torrent->status.handle.trackers()
    });
}
