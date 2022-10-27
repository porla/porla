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
        return cb.Error(-1, "Torrent not found");
    }

    std::vector<lt::peer_info> peers;
    status->second.handle.get_peer_info(peers);

    cb.Ok(TorrentsPeersListRes{
        .peers = peers
    });
}
