#include "torrentspeerslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPeersList;

TorrentsPeersList::TorrentsPeersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPeersList::Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb)
{
    const auto& state = std::find_if(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        [hash = req.info_hash](const auto& state)
        {
            return state.second->torrents.find(hash) != state.second->torrents.end();
        });

    if (state == m_sessions.All().end())
    {
        return cb.Error(-1, "Torrent not found in any session");
    }

    const auto& handle = state->second->torrents.find(req.info_hash);

    if (handle == state->second->torrents.end())
    {
        return cb.Error(-1, "Torrent not found");
    }

    std::vector<lt::peer_info> peers;
    handle->second.get_peer_info(peers);

    cb.Ok(TorrentsPeersListRes{
        .peers = peers
    });
}
