#include "torrentspeerslist.hpp"

#include "../sessions.hpp"

using porla::Methods::TorrentsPeersList;

template <bool SSL> TorrentsPeersList<SSL>::TorrentsPeersList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void TorrentsPeersList<SSL>::Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes, SSL> cb)
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

    const auto& [ th, _ ] = handle->second;

    if (!th.is_valid())
    {
        return cb.Error(-2, "Torrent not valid");
    }

    std::vector<lt::peer_info> peers;
    th.get_peer_info(peers);

    cb.Ok(TorrentsPeersListRes{
        .peers = peers
    });
}

namespace porla::Methods {
    template class TorrentsPeersList<true>;
    template class TorrentsPeersList<false>;
}
