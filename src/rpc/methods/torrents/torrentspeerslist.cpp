#include "torrentspeerslist.hpp"

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPeersList;

TorrentsPeersList::TorrentsPeersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPeersList::Execute(const TorrentsPeersListReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    std::vector<lt::peer_info> peers;
    resolved->torrent->status.handle.get_peer_info(peers);

    cb->Ok(TorrentsPeersListRes{
        .peers = peers
    });
}
