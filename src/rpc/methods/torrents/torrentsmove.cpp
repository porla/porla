#include "torrentsmove.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsMove;
using porla::Rpc::Methods::Torrents::TorrentsMoveReq;
using porla::Rpc::Methods::Torrents::TorrentsMoveRes;

TorrentsMove::TorrentsMove(porla::Sessions &sessions)
    : m_sessions(sessions)
{
}

void TorrentsMove::Execute(const TorrentsMoveReq &req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    lt::move_flags_t flags = lt::move_flags_t::dont_replace;

    if (req.flags.has_value())
    {
        if (req.flags.value() == "always_replace_files") flags = lt::move_flags_t::always_replace_files;
        if (req.flags.value() == "dont_replace")         flags = lt::move_flags_t::dont_replace;
        if (req.flags.value() == "fail_if_exist")        flags = lt::move_flags_t::fail_if_exist;
    }

    if (!resolved->torrent->status.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    resolved->torrent->status.handle.move_storage(req.path, flags);

    return cb->Ok(TorrentsMoveRes{});
}
