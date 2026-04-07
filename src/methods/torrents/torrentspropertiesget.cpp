#include "torrentspropertiesget.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsPropertiesGet;
using porla::Methods::TorrentsPropertiesGetReq;
using porla::Methods::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesGet::Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb)
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

    cb.Ok(TorrentsPropertiesGetRes{
        .download_limit  = th.download_limit(),
        .flags           = th.flags(),
        .max_connections = th.max_connections(),
        .max_uploads     = th.max_uploads(),
        .upload_limit    = th.upload_limit()
    });
}
