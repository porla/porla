#include "session.hpp"

#include <boost/log/trivial.hpp>

#include "data/models/addtorrentparams.hpp"

namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
using porla::Session;

Session::Session(boost::asio::io_context& io, porla::SessionOptions const& options)
    : m_io(io)
    , m_db(options.db)
    , m_timer(io)
{
    m_session = std::make_unique<lt::session>();
    m_session->set_alert_notify(
        [this]()
        {
            boost::asio::post(m_io, [this] { ReadAlerts(); });
        });

    boost::system::error_code ec;
    m_timer.expires_from_now(boost::posix_time::seconds(1), ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec;
    }
    else
    {
        m_timer.async_wait(
            [this](auto &&PH1)
            {
                PostUpdates(std::forward<decltype(PH1)>(PH1));
            });
    }
}

Session::~Session()
{
    BOOST_LOG_TRIVIAL(info) << "Shutting down session";

    m_session->set_alert_notify([]{});
    m_timer.cancel();


}

void Session::Load()
{
    BOOST_LOG_TRIVIAL(info) << "Loading " << AddTorrentParams::Count(m_db) << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_db,
        [&](const lt::add_torrent_params& params)
        {
            m_session->async_add_torrent(params);
        });
}

void Session::AddTorrent(lt::add_torrent_params const& p)
{
    m_session->async_add_torrent(p);
}

const std::map<lt::info_hash_t, lt::torrent_status>& Session::Torrents()
{
    return m_torrents;
}

void Session::ReadAlerts()
{
    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);

    for (auto const alert : alerts)
    {
        BOOST_LOG_TRIVIAL(trace) << alert->message();
    }
}

void Session::PostUpdates(boost::system::error_code ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec;
        return;
    }

    m_session->post_dht_stats();
    m_session->post_session_stats();
    m_session->post_torrent_updates();

    m_timer.expires_from_now(boost::posix_time::seconds(1), ec);
    m_timer.async_wait([this](auto && PH1) { PostUpdates(std::forward<decltype(PH1)>(PH1)); });
}
