#include "torrentsget.hpp"

#include "resolve.hpp"

#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsGet;
using porla::Rpc::Methods::Torrents::TorrentsGetReq;
using porla::Rpc::Methods::Torrents::TorrentsGetRes;

TorrentsGet::TorrentsGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsGet::Execute(const TorrentsGetReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    return cb->Ok(TorrentsGetRes{
        .torrent = resolved->torrent->status
    });
}
