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
    const auto& state = std::find_if(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        [hash = req.info_hash](const auto& state)
        {
            return state.second->torrents.find(hash) != state.second->torrents.end();
        });

    if (state == m_sessions.All().end())
    {
        return cb.Error(-1, "Torrent not found in any session");
    }

    const auto& handle = state->second->torrents.find(req.info_hash);

    if (handle == state->second->torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    const auto& [ th, _ ] = handle->second;

    cb.Ok(TorrentsTrackersListRes{
        .trackers = th.trackers()
    });
}
