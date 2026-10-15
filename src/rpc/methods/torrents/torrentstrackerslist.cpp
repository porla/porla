#include "torrentstrackerslist.hpp"

#include "../resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

struct TorrentsTrackersListRes
{
    std::vector<libtorrent::announce_entry> trackers;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
    TorrentsTrackersListRes,
    trackers)

using porla::Rpc::Methods::Torrents::TorrentsTrackersList;

TorrentsTrackersList::TorrentsTrackersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsTrackersList::Execute(const TorrentRef& req, ResponseWriterHandle cb)
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
