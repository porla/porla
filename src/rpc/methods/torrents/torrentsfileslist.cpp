#include "torrentsfileslist.hpp"

#include <algorithm>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesList;
using porla::Rpc::Methods::Torrents::TorrentsFilesListReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsFilesList::Execute(const TorrentsFilesListReq& req, ResponseWriterHandle cb)
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

    const auto torrent = session_state->Find(req.info_hash);

    if (torrent == nullptr)
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    if (auto tf = torrent->status.torrent_file.lock())
    {
        return cb->Ok(TorrentsFilesListRes{
            .file_storage  = tf->layout(),
            .renamed_files = torrent->status.handle.get_renamed_files()
        });
    }

    return cb->Error(-4, "Failed to lock torrent file");
}
