#include "torrentstrackerslist.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsTrackersList;
using porla::Methods::TorrentsTrackersListReq;
using porla::Methods::TorrentsTrackersListRes;

TorrentsTrackersList::TorrentsTrackersList(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsTrackersList::Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const torrent = torrents.find(req.info_hash);

    if (torrent == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    if (!torrent->second.handle.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    cb.Ok(TorrentsTrackersListRes{
        .trackers = torrent->second.handle.trackers()
    });
}
