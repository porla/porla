#include "torrentstrackerslist.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsTrackersList;
using porla::Methods::TorrentsTrackersListReq;
using porla::Methods::TorrentsTrackersListRes;

TorrentsTrackersList::TorrentsTrackersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsTrackersList::Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb)
{
    const auto& session_state = req.session_id.has_value()
        ? m_sessions.Get(req.session_id.value())
        : m_sessions.Default();

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb.Error(-2, "Torrent not found in session");
    }

    const auto& [ th, _ ] = handle->second;

    if (!th.is_valid())
    {
        return cb.Error(-3, "Torrent not valid");
    }

    cb.Ok(TorrentsTrackersListRes{
        .trackers = th.trackers()
    });
}
