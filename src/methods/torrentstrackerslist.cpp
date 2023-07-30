#include "torrentstrackerslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsTrackersList;
using porla::Methods::TorrentsTrackersListReq;
using porla::Methods::TorrentsTrackersListRes;

TorrentsTrackersList::TorrentsTrackersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsTrackersList::Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto const torrent = torrents.find(req.info_hash);

    if (torrent == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    if (!torrent->second.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    cb.Ok(TorrentsTrackersListRes{
        .trackers = torrent->second.trackers()
    });
}
