#include "session.hpp"

#include <boost/log/trivial.hpp>

#include "data/models/addtorrentparams.hpp"

namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
using porla::Session;

Session::Session(boost::asio::io_context& io, porla::SessionOptions const& options)
    : m_io(io)
    , m_db(options.db)
{
    m_session = std::make_unique<lt::session>();
    m_session->set_alert_notify([]{});
}

Session::~Session()
{
    m_session->set_alert_notify([]{});
}

void Session::Load()
{
    BOOST_LOG_TRIVIAL(info) << "Loading " << AddTorrentParams::Count(m_db) << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_db,
        [](const lt::add_torrent_params& params)
        {
        });
}

void Session::AddTorrent(lt::add_torrent_params const& p)
{
}

const std::map<lt::info_hash_t, lt::torrent_status>& Session::Torrents()
{
    return m_torrents;
}
