#include "torrentsget.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsGet;
using porla::Methods::TorrentsGetReq;
using porla::Methods::TorrentsGetRes;

TorrentsGet::TorrentsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsGet::Invoke(const TorrentsGetReq& req, WriteCb<TorrentsGetRes> cb)
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

    const auto& [ _, status ] = handle->second;

    return cb.Ok(TorrentsGetRes{
        .torrent = status
    });
}
