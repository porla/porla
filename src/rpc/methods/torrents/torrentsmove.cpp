#include "torrentsmove.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Torrents::TorrentsMove;
using porla::Rpc::Methods::Torrents::TorrentsMoveReq;
using porla::Rpc::Methods::Torrents::TorrentsMoveRes;

TorrentsMove::TorrentsMove(sqlite3* db, porla::Sessions &sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsMove::Execute(const TorrentsMoveReq &req, ResponseWriterHandle cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb->Error(-3, "Torrent not found in session");
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
        return cb->Error(-4, "Torrent not valid");
    }

    th.move_storage(req.path, flags);

    return cb->Ok(TorrentsMoveRes{});
}
