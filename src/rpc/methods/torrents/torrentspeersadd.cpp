#include "torrentspeersadd.hpp"

#include <boost/log/trivial.hpp>

#include "resolve.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPeersAdd;
using porla::Rpc::Methods::Torrents::TorrentsPeersAddReq;
using porla::Rpc::Methods::Torrents::TorrentsPeersAddRes;

TorrentsPeersAdd::TorrentsPeersAdd(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsPeersAdd::Execute(const TorrentsPeersAddReq& req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    for (const auto& [ip, port] : req.peers)
    {
        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(ip, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse '" << ip << "': " << ec.message();
            continue;
        }

        resolved->torrent->status.handle.connect_peer(boost::asio::ip::tcp::endpoint{addr,port});
    }

    cb->Ok(TorrentsPeersAddRes{});
}
