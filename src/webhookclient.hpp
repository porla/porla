#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <nlohmann/json.hpp>

#include "config.hpp"

namespace porla
{
    class ISession;

    struct WebhookClientOptions
    {
        ISession& session;
        std::vector<Config::Webhook> webhooks;
    };

    class WebhookClient
    {
    public:
        explicit WebhookClient(boost::asio::io_context& io, const WebhookClientOptions& opts);
        ~WebhookClient();

    private:
        struct RequestState;

        void OnAsyncConnect(
            boost::system::error_code ec,
            const boost::asio::ip::tcp::resolver::results_type::endpoint_type& type,
            std::shared_ptr<RequestState> state);

        void OnAsyncHandshake(boost::system::error_code ec, std::shared_ptr<RequestState> state);
        void OnAsyncRead(boost::system::error_code ec, std::size_t size, std::shared_ptr<RequestState> state);
        void OnAsyncResolve(
            boost::system::error_code ec,
            const boost::asio::ip::tcp::resolver::results_type& results,
            std::shared_ptr<RequestState> state);
        void OnAsyncWrite(boost::system::error_code ec, std::size_t size, std::shared_ptr<RequestState> state);

        void OnTorrentAdded(const libtorrent::torrent_status& ts);
        void OnTorrentFinished(const libtorrent::torrent_status& ts);
        void OnTorrentPaused(const libtorrent::torrent_status& ts);
        void OnTorrentRemoved(const libtorrent::info_hash_t& ih);
        void OnTorrentResumed(const libtorrent::torrent_status& ts);

        void SendEvent(const std::string& eventName, const std::map<std::string, nlohmann::json>& ext_vars);
        void SendRequest(std::shared_ptr<RequestState> state);

        boost::asio::io_context& m_io;
        boost::asio::ip::tcp::resolver m_resolver;

        ISession& m_session;
        std::vector<Config::Webhook> m_webhooks;

        boost::signals2::connection m_torrentAddedConnection;
        boost::signals2::connection m_torrentFinishedConnection;
        boost::signals2::connection m_torrentPausedConnection;
        boost::signals2::connection m_torrentRemovedConnection;
        boost::signals2::connection m_torrentResumedConnection;
    };
}
