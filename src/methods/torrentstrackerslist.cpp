#include "torrentstrackerslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsTrackersList;
using porla::Methods::TorrentsTrackersListReq;
using porla::Methods::TorrentsTrackersListRes;

template <bool SSL> TorrentsTrackersList<SSL>::TorrentsTrackersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void TorrentsTrackersList<SSL>::Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes, SSL> cb)
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

    if (!th.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    cb.Ok(TorrentsTrackersListRes{
        .trackers = th.trackers()
    });
}

namespace porla::Methods {
    template class TorrentsTrackersList<true>;
    template class TorrentsTrackersList<false>;
}
