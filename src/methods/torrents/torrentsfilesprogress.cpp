#include "torrentsfilesprogress.hpp"

#include <algorithm>

#include "../../sessions.hpp"

using porla::Methods::TorrentsFilesProgress;
using porla::Methods::TorrentsFilesProgressReq;
using porla::Methods::TorrentsFilesProgressRes;

TorrentsFilesProgress::TorrentsFilesProgress(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesProgress::Invoke(const TorrentsFilesProgressReq& req, WriteCb<TorrentsFilesProgressRes> cb)
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

    const auto& [ th, _ ] = handle->second;

    TorrentsFilesProgressRes res;
    th.file_progress(res.progress);

    return cb.Ok(res);
}
