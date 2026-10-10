#include "torrentspause.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPause;
using porla::Rpc::Methods::Torrents::TorrentsPauseReq;
using porla::Rpc::Methods::Torrents::TorrentsPauseRes;

TorrentsPause::TorrentsPause(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPause::Execute(const TorrentsPauseReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    resolved->torrent->status.handle.unset_flags(lt::torrent_flags::auto_managed);
    resolved->torrent->status.handle.pause();

    cb->Ok(TorrentsPauseRes{});
}
