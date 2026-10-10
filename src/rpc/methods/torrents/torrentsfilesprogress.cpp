#include "torrentsfilesprogress.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesProgress;
using porla::Rpc::Methods::Torrents::TorrentsFilesProgressReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesProgressRes;

TorrentsFilesProgress::TorrentsFilesProgress(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsFilesProgress::Execute(const TorrentsFilesProgressReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    TorrentsFilesProgressRes res;
    resolved->torrent->status.handle.file_progress(res.progress);

    cb->Ok(res);
}
