#include "torrentsresume.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsResume;
using porla::Rpc::Methods::Torrents::TorrentsResumeReq;
using porla::Rpc::Methods::Torrents::TorrentsResumeRes;

TorrentsResume::TorrentsResume(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsResume::Execute(const TorrentsResumeReq& req, ResponseWriterHandle cb)
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
        return cb->Error(-3, "Torrent not found in session");
    }

    if (!it->second.status.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    it->second.status.handle.resume();

    cb->Ok(TorrentsResumeRes{});
}
