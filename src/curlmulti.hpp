#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>
#include <curl/curl.h>

// NOTE: POSIX only (uses dup() and boost::asio::posix::stream_descriptor).

namespace porla
{
    class CurlMulti : public std::enable_shared_from_this<CurlMulti>
    {
    public:
        // Invoked exactly once for every handle passed to AddTransfer(),
        // including on cancellation and on shutdown.
        //
        //  - `result` is the *transfer level* result. CURLE_OK does not imply
        //    HTTP 200 - use curl_easy_getinfo(CURLINFO_RESPONSE_CODE) for that.
        //  - `easy` has already been removed from the multi handle and is valid
        //    for curl_easy_getinfo() for the duration of the callback only.
        //    Do NOT call curl_easy_cleanup() on it; CurlMulti owns it.
        using TransferComplete = std::function<void(CURL* easy, CURLcode result)>;

        // CurlMulti must be owned by a shared_ptr: in-flight completion
        // handlers hold a weak_ptr so that they degrade to a no-op if the
        // instance is destroyed while handlers are still queued on the
        // io_context.
        static std::shared_ptr<CurlMulti> Create(boost::asio::io_context& io);

        // Must be destroyed either from the thread running the io_context, or
        // after the io_context has been stopped and all threads joined.
        ~CurlMulti();

        CurlMulti(const CurlMulti&)            = delete;
        CurlMulti& operator=(const CurlMulti&) = delete;
        CurlMulti(CurlMulti&&)                 = delete;
        CurlMulti& operator=(CurlMulti&&)      = delete;

        // Takes ownership of `easy`. Safe to call from any thread; the work is
        // dispatched onto the internal strand.
        void AddTransfer(CURL* easy, TransferComplete callback);

        boost::asio::any_io_executor Executor() const { return m_strand; }

        // Aborts a transfer previously handed to AddTransfer(). Its callback is
        // invoked with CURLE_ABORTED_BY_CALLBACK. No-op if unknown/finished.
        // Safe to call from any thread.
        void CancelTransfer(CURL* easy);

        // Aborts everything and releases the multi handle. Idempotent. Called
        // by the destructor; call it earlier for a deterministic shutdown.
        // Must run on the strand (see destructor note above).
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

        boost::asio::io_context&                                 m_io;
        boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
        boost::asio::steady_timer                                m_timer;
        CURLM*                                                   m_multi;

        std::uint64_t m_timer_generation;
        int           m_running;
        bool          m_shutdown;
        bool          m_in_check_completed;

        std::unordered_map<CURL*, TransferComplete>                     m_transfers;
        std::unordered_map<curl_socket_t, std::unique_ptr<SocketState>> m_sockets;
    };
}
