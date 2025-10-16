#include "torrentsfileslist.hpp"

#include <algorithm>

#include "../../sessions.hpp"

using porla::Methods::TorrentsFilesList;
using porla::Methods::TorrentsFilesListReq;
using porla::Methods::TorrentsFilesListRes;

TorrentsFilesList::TorrentsFilesList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesList::Invoke(const TorrentsFilesListReq& req, WriteCb<TorrentsFilesListRes> cb)
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

    const auto& [ _, status ] = handle->second;

    TorrentsFilesListRes res;

    if (auto tf = status.torrent_file.lock())
    {
        return cb.Ok(TorrentsFilesListRes{
            .file_storage = tf->files()
        });
    }

    return cb.Error(-2, "Failed to lock torrent file");
}
