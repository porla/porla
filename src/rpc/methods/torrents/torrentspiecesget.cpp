#include "torrentspiecesget.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPiecesGet;

TorrentsPiecesGet::TorrentsPiecesGet(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPiecesGet::Execute(const TorrentsPiecesGetReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    cb->Ok(TorrentsPiecesGetRes{
        .pieces          = resolved->torrent->status.pieces,
        .verified_pieces = resolved->torrent->status.verified_pieces
    });
}
