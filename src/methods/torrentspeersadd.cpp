#include "torrentspeersadd.hpp"

#include <boost/log/trivial.hpp>

#include "../sessions.hpp"

using porla::Methods::TorrentsPeersAdd;
using porla::Methods::TorrentsPeersAddReq;
using porla::Methods::TorrentsPeersAddRes;

template <bool SSL> TorrentsPeersAdd<SSL>::TorrentsPeersAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

template <bool SSL> void TorrentsPeersAdd<SSL>::Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes, SSL> cb)
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

    for (const auto& [ip,port] : req.peers)
    {
        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(ip, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse '" << ip << "': " << ec.message();
            continue;
        }

        th.connect_peer(boost::asio::ip::tcp::endpoint{addr,port});
    }

    cb(TorrentsPeersAddRes{});
}

namespace porla::Methods {
    template class TorrentsPeersAdd<true>;
    template class TorrentsPeersAdd<false>;
}
