#include "torrentspeersadd.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/sessions.hpp"
#include "../../sessions.hpp"

using porla::Methods::TorrentsPeersAdd;
using porla::Methods::TorrentsPeersAddReq;
using porla::Methods::TorrentsPeersAddRes;

TorrentsPeersAdd::TorrentsPeersAdd(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsPeersAdd::Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes> cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb.Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb.Error(-2, "Session not loaded");
    }

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb.Error(-3, "Torrent not found in session");
    }

    const auto& [ th, _ ] = handle->second;

    if (!th.is_valid())
    {
        return cb.Error(-4, "Torrent not valid");
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

        th.connect_peer(boost::asio::ip::tcp::endpoint{addr,port});
    }

    cb(TorrentsPeersAddRes{});
}
