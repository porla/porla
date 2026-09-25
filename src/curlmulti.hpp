#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <curl/curl.h>

namespace porla
{
    class CurlMulti : public std::enable_shared_from_this<CurlMulti>
    {
    public:
        using HttpCallback     = boost::asio::any_completion_handler<void(int, std::string)>;
        using TransferComplete = boost::asio::any_completion_handler<void(CURL* easy, CURLcode result)>;

        static std::shared_ptr<CurlMulti> Create(boost::asio::io_context& io);

        ~CurlMulti();

        CurlMulti(const CurlMulti&)            = delete;
        CurlMulti& operator=(const CurlMulti&) = delete;
        CurlMulti(CurlMulti&&)                 = delete;
        CurlMulti& operator=(CurlMulti&&)      = delete;

        void AddTransfer(CURL* easy, TransferComplete callback);

        boost::asio::any_io_executor Executor() const { return m_strand; }

        void HttpGet(const std::string& url, HttpCallback callback);

        template<boost::asio::completion_token_for<void(int, std::string)> CompletionToken>
        auto AsyncHttpGet(std::string url, CompletionToken&& token)
        {
            return boost::asio::async_initiate<CompletionToken, void(int, std::string)>(
                [](auto handler, std::shared_ptr<CurlMulti> self, std::string url)
                {
                    self->HttpGet(url, std::move(handler));
                },
                token,
                shared_from_this(),
                std::move(url));
        }

        void CancelTransfer(CURL* easy);

        void Shutdown();

    private:
        struct SocketState;

        explicit CurlMulti(boost::asio::io_context& io);

        static void SafeInvoke(TransferComplete&& callback, CURL* easy, CURLcode result);
        static int SocketCallback(CURL* easy, curl_socket_t sock, int what, void* userp, void* socketp);
        static int TimerCallback(CURLM* multi, long timeout_ms, void* userp);

        void DoAddTransfer(CURL* easy, TransferComplete callback);
        void ArmSocket(curl_socket_t sock);
        void DriveSocket(curl_socket_t sock, int mask);
        void OnSocketReady(curl_socket_t sock, const boost::system::error_code& ec, int direction);
        void OnSocketUpdate(curl_socket_t sock, int what);
        void OnTimeout(std::uint64_t generation);
        void SocketAction(curl_socket_t sock, int event_bitmask);
        void CheckCompleted();
        void FinishTransfer(CURL* easy, CURLcode result);
        bool StillReadable(curl_socket_t sock); 

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
