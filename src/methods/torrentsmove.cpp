#include "torrentsmove.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsMove;
using porla::Methods::TorrentsMoveReq;
using porla::Methods::TorrentsMoveRes;

TorrentsMove::TorrentsMove(porla::Sessions &sessions)
    : m_sessions(sessions)
{
}

void TorrentsMove::Invoke(const TorrentsMoveReq &req, WriteCb<TorrentsMoveRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto const& handle = torrents.find(req.info_hash);

    if (handle == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    lt::move_flags_t flags = lt::move_flags_t::dont_replace;

    if (req.flags.has_value())
    {
        if (req.flags.value() == "always_replace_files") flags = lt::move_flags_t::always_replace_files;
        if (req.flags.value() == "dont_replace")         flags = lt::move_flags_t::dont_replace;
        if (req.flags.value() == "fail_if_exist")        flags = lt::move_flags_t::fail_if_exist;
    }

    handle->second.move_storage(req.path, flags);

    return cb.Ok(TorrentsMoveRes{});
}
