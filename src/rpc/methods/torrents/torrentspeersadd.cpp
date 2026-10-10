#include "torrentspeersadd.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPeersAdd;
using porla::Rpc::Methods::Torrents::TorrentsPeersAddReq;
using porla::Rpc::Methods::Torrents::TorrentsPeersAddRes;

TorrentsPeersAdd::TorrentsPeersAdd(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsPeersAdd::Execute(const TorrentsPeersAddReq& req, ResponseWriterHandle cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    const auto torrent = session_state->Find(req.info_hash);

    if (torrent == nullptr)
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    if (!torrent->status.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
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

        torrent->status.handle.connect_peer(boost::asio::ip::tcp::endpoint{addr,port});
    }

    cb->Ok(TorrentsPeersAddRes{});
}
