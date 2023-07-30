#include "torrentspeersadd.hpp"

#include <boost/log/trivial.hpp>

#include "../sessions.hpp"

using porla::Methods::TorrentsPeersAdd;
using porla::Methods::TorrentsPeersAddReq;
using porla::Methods::TorrentsPeersAddRes;

TorrentsPeersAdd::TorrentsPeersAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPeersAdd::Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes> cb)
{
    auto const& torrents = m_sessions.Default()->torrents;
    auto status = torrents.find(req.info_hash);

    if (status != torrents.end())
    {
        for (auto const& [ip,port] : req.peers)
        {
            boost::system::error_code ec;
            auto addr = boost::asio::ip::make_address(ip, ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to parse '" << ip << "': " << ec.message();
                continue;
            }

            status->second.connect_peer(
                boost::asio::ip::tcp::endpoint{addr,port});
        }
    }

    cb(TorrentsPeersAddRes{});
}
