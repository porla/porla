#include "torrentsfilesprogress.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesProgress;
using porla::Rpc::Methods::Torrents::TorrentsFilesProgressReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesProgressRes;

TorrentsFilesProgress::TorrentsFilesProgress(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsFilesProgress::Execute(const TorrentsFilesProgressReq& req, ResponseWriterHandle cb)
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

    const auto& [ th, _ ] = handle->second;

    TorrentsFilesProgressRes res;
    th.file_progress(res.progress);

    cb->Ok(res);
}
