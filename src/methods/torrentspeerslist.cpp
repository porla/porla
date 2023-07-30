#include "torrentspeerslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPeersList;

TorrentsPeersList::TorrentsPeersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPeersList::Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto const& status = torrents.find(req.info_hash);

    if (status == torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    std::vector<lt::peer_info> peers;
    status->second.get_peer_info(peers);

    cb.Ok(TorrentsPeersListRes{
        .peers = peers
    });
}
