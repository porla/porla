#include "torrentsresume.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsResume;
using porla::Rpc::Methods::Torrents::TorrentsResumeReq;
using porla::Rpc::Methods::Torrents::TorrentsResumeRes;

TorrentsResume::TorrentsResume(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsResume::Execute(const TorrentsResumeReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->torrent->status.handle.resume();

    cb->Ok(TorrentsResumeRes{});
}
