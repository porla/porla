#include "torrentspeerslist.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsPeersList;

TorrentsPeersList::TorrentsPeersList(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsPeersList::Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb)
{
    auto const& torrents = m_session.Torrents();
    auto const& status = torrents.find(req.info_hash);

    if (status == torrents.end())
    {
        cb(TorrentsPeersListRes{}); // TODO: Return error
        return;
    }

    std::vector<lt::peer_info> peers;
    status->second.handle.get_peer_info(peers);

    TorrentsPeersListRes res;

    for (auto const& peer : peers)
    {
        res.peers.push_back({
            .client = peer.client,
            .down_speed = peer.down_speed,
            .up_speed = peer.up_speed
        });
    }

    cb(res);
}
