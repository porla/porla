#include "torrentsremove.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsRemove;
using porla::Methods::TorrentsRemoveReq;
using porla::Methods::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRemove::Invoke(const TorrentsRemoveReq &req, WriteCb<TorrentsRemoveRes> cb)
{
    const auto& session_state = req.session_id.has_value()
        ? m_sessions.Get(req.session_id.value())
        : m_sessions.Default();

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    for (const auto& hash : req.info_hashes)
    {
        const auto& handle = session_state->torrents.find(hash);

        if (handle == session_state->torrents.end())
        {
            continue;
        }

        const auto& [ th, _ ] = handle->second;

        if (!th.is_valid())
        {
            continue;
        }

        session_state->session->remove_torrent(
            th,
            req.remove_data ? lt::session::delete_files : lt::remove_flags_t{});
    }

    cb(TorrentsRemoveRes{});
}
