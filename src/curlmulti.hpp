#pragma once

#include <boost/asio.hpp>
#include <curl/curl.h>

namespace porla
{
    class CurlMulti
    {
    public:
        using TransferComplete = std::function<void(CURL* easy)>;

        explicit CurlMulti(boost::asio::io_context& io);
        ~CurlMulti();

        void AddTransfer(CURL* easy, TransferComplete callback);

    private:
        static int SocketCallback(CURL* easy, curl_socket_t s, int action, void* userp, void* socketp);
        static int TimerCallback(CURLM* multi, long timeout_ms, void* userp);

        void WatchSocket(curl_socket_t s, int action);
        void OnSocketReady(curl_socket_t s, boost::system::error_code ec, int action);
        void OnTimeout(boost::system::error_code ec);
        void CheckCompleted();

        boost::asio::io_context& m_io;
        CURLM* m_multi;
        std::unordered_map<curl_socket_t, std::unique_ptr<boost::asio::posix::stream_descriptor>> m_sockets;
        boost::asio::steady_timer m_timer;
        std::unordered_map<CURL*, TransferComplete> m_transfers;
    };
}
