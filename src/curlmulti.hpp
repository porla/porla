#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <curl/curl.h>

namespace porla
{
    class CurlMulti : public std::enable_shared_from_this<CurlMulti>
    {
    public:
        using HttpCallback     = std::function<void(int, std::string)>;
        using TransferComplete = std::function<void(CURL* easy, CURLcode result)>;

        static std::shared_ptr<CurlMulti> Create(boost::asio::io_context& io);

        ~CurlMulti();

        CurlMulti(const CurlMulti&)            = delete;
        CurlMulti& operator=(const CurlMulti&) = delete;
        CurlMulti(CurlMulti&&)                 = delete;
        CurlMulti& operator=(CurlMulti&&)      = delete;

        void AddTransfer(CURL* easy, TransferComplete callback);

        boost::asio::any_io_executor Executor() const { return m_strand; }

        void HttpGet(const std::string& url, HttpCallback callback);

        void CancelTransfer(CURL* easy);

        void Shutdown();

    private:
        struct SocketState;

        explicit CurlMulti(boost::asio::io_context& io);

        static int SocketCallback(CURL* easy, curl_socket_t sock, int what, void* userp, void* socketp);
        static int TimerCallback(CURLM* multi, long timeout_ms, void* userp);

        void DoAddTransfer(CURL* easy, TransferComplete callback);
        void OnSocketUpdate(curl_socket_t sock, int what);
        void ArmSocket(curl_socket_t sock);
        void OnSocketReady(curl_socket_t sock, const boost::system::error_code& ec, int direction);
        void OnTimeout(std::uint64_t generation);
        void SocketAction(curl_socket_t sock, int event_bitmask);
        void CheckCompleted();
        void FinishTransfer(CURL* easy, CURLcode result);
        void SafeInvoke(const TransferComplete& callback, CURL* easy, CURLcode result);

        boost::asio::io_context&                                    m_io;
        boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
        boost::asio::steady_timer                                   m_timer;
        CURLM*                                                      m_multi;

        std::uint64_t m_timer_generation;
        int           m_running;
        bool          m_shutdown;
        bool          m_in_check_completed;

        std::unordered_map<CURL*, TransferComplete>                     m_transfers;
        std::unordered_map<curl_socket_t, std::unique_ptr<SocketState>> m_sockets;
    };
}
