#include "torrentsfileslist.hpp"

#include <algorithm>

#include "../../data/models/sessions.hpp"
#include "../../sessions.hpp"

using porla::Methods::TorrentsFilesList;
using porla::Methods::TorrentsFilesListReq;
using porla::Methods::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsFilesList::Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb.Error(-2, "Session not loaded");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb.Error(-3, "Torrent not found in session");
    }

    const auto& [ _, status ] = handle->second;

    if (auto tf = status.torrent_file.lock())
    {
        return cb.Ok(TorrentsFilesListRes{
            .file_storage = tf->files()
        });
    }

    return cb.Error(-4, "Failed to lock torrent file");
}
