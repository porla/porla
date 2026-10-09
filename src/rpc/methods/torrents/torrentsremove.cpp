#include "torrentsremove.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsRemove;
using porla::Rpc::Methods::Torrents::TorrentsRemoveReq;
using porla::Rpc::Methods::Torrents::TorrentsRemoveRes;

TorrentsRemove::TorrentsRemove(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsRemove::Execute(const TorrentsRemoveReq &req, ResponseWriterHandle cb)
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

    const auto it = session_state->Torrents().find(req.info_hash);

    if (it == session_state->Torrents().end())
    {
        return cb->Error(-3, "Torrent not found");
    }

    if (!it->second.status.handle.is_valid())
    {
        return cb->Error(-4, "Invalid torrent handle");
    }

    session_state->Libtorrent().remove_torrent(
        it->second.status.handle,
        req.remove_data.value_or(false)
            ? lt::session::delete_files
            : lt::remove_flags_t{});

    cb->Ok(TorrentsRemoveRes{});
}
