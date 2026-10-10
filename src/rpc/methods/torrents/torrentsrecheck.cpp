#include "torrentsrecheck.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsRecheck;
using porla::Rpc::Methods::Torrents::TorrentsRecheckReq;
using porla::Rpc::Methods::Torrents::TorrentsRecheckRes;

TorrentsRecheck::TorrentsRecheck(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsRecheck::Execute(const TorrentsRecheckReq &req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->session->Recheck(resolved->torrent->status.info_hashes);

    return cb->Ok(TorrentsRecheckRes{});
}
