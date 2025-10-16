#include "torrentsmove.hpp"

#include "../../sessions.hpp"

using porla::Methods::TorrentsMove;
using porla::Methods::TorrentsMoveReq;
using porla::Methods::TorrentsMoveRes;

TorrentsMove::TorrentsMove(porla::Sessions &sessions)
    : m_sessions(sessions)
{
}

void TorrentsMove::Invoke(const TorrentsMoveReq &req, WriteCb<TorrentsMoveRes> cb)
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

    lt::move_flags_t flags = lt::move_flags_t::dont_replace;

    if (req.flags.has_value())
    {
        if (req.flags.value() == "always_replace_files") flags = lt::move_flags_t::always_replace_files;
        if (req.flags.value() == "dont_replace")         flags = lt::move_flags_t::dont_replace;
        if (req.flags.value() == "fail_if_exist")        flags = lt::move_flags_t::fail_if_exist;
    }

    const auto& [ th, _ ] = handle->second;

    if (!th.is_valid())
    {
        return cb.Error(-3, "Torrent not valid");
    }

    th.move_storage(req.path, flags);

    return cb.Ok(TorrentsMoveRes{});
}
