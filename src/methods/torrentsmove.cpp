#include "torrentsmove.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsMove;
using porla::Methods::TorrentsMoveReq;
using porla::Methods::TorrentsMoveRes;

TorrentsMove::TorrentsMove(porla::ISession &session)
    : m_session(session)
{
}

void TorrentsMove::Invoke(const TorrentsMoveReq &req, WriteCb<TorrentsMoveRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const& status = torrents.find(req.info_hash);

    if (status == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    lt::move_flags_t flags = lt::move_flags_t::always_replace_files;

    if (req.flags.has_value())
    {
        if (req.flags.value() == "always_replace_files") flags = lt::move_flags_t::always_replace_files;
        if (req.flags.value() == "dont_replace")         flags = lt::move_flags_t::dont_replace;
        if (req.flags.value() == "fail_if_exist")        flags = lt::move_flags_t::fail_if_exist;
    }

    status->second.handle.move_storage(req.path, flags);
}
