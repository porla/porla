#include "torrentspropertiesset.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsPropertiesSet;
using porla::Methods::TorrentsPropertiesSetReq;
using porla::Methods::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPropertiesSet::Invoke(const TorrentsPropertiesSetReq& req, WriteCb<TorrentsPropertiesSetRes> cb)
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

    if (const auto val = req.download_limit)
        th.set_download_limit(*val);

    if (const auto val = req.flags)
    {
        lt::torrent_flags_t mask  = {};
        th.set_flags(*val, mask);
    }

    if (const auto val = req.max_connections)
        th.set_max_connections(*val);

    if (const auto val = req.max_uploads)
        th.set_max_uploads(*val);

    if (const auto val = req.upload_limit)
        th.set_upload_limit(*val);

    cb.Ok({});
}
