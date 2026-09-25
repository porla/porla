#include "curlmulti.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <unistd.h>

#include <boost/log/trivial.hpp>

#include "buildinfo.hpp"

namespace
{
    static constexpr std::size_t kMaxResponseBytes = 8 * 1024 * 1024;

    template<typename Executor>
    void CompleteHttp(
        boost::asio::io_context&       io,
        const Executor&                ex,
        porla::CurlMulti::HttpCallback callback,
        int                            status,
        std::string                    body)
    {
        boost::asio::post(
            io,
            boost::asio::bind_executor(
                ex,
                [callback = std::move(callback), status, body = std::move(body)]() mutable
                {
                    std::move(callback)(status, std::move(body));
                }));
    }
}

using porla::CurlMulti;

static size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    const size_t total  = size * nmemb;
    auto*        stream = static_cast<std::stringstream*>(userdata);

    if (static_cast<std::size_t>(stream->tellp()) + total > kMaxResponseBytes)
    {
        return 0;
    }

    stream->write(ptr, total);

    return total;
}

struct CurlMulti::SocketState
{
    SocketState(const boost::asio::any_io_executor& ex, int fd)
        : descriptor(ex, fd) {}

    boost::asio::posix::stream_descriptor descriptor;

    int  action  = 0;      // last CURL_POLL_* mask libcurl asked for
    bool reading = false;  // an async_wait(wait_read) is in flight
    bool writing = false;  // an async_wait(wait_write) is in flight
};

std::shared_ptr<CurlMulti> CurlMulti::Create(boost::asio::io_context& io)
{
    // Private constructor, so no make_shared.
    return std::shared_ptr<CurlMulti>(new CurlMulti(io));
}

CurlMulti::CurlMulti(boost::asio::io_context& io)
    : m_io(io)
    , m_strand(boost::asio::make_strand(io))
    , m_timer(m_strand)
    , m_multi(curl_multi_init())
    , m_timer_generation(0)
    , m_running(0)
    , m_shutdown(false)
    , m_in_check_completed(false)
{
    if (m_multi == nullptr)
    {
        throw std::runtime_error("curl_multi_init failed");
    }

    curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, &CurlMulti::SocketCallback);
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA,     this);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERFUNCTION,  &CurlMulti::TimerCallback);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA,      this);
}

CurlMulti::~CurlMulti()
{
    Shutdown();
}

void CurlMulti::Shutdown()
{
    if (m_shutdown)
    {
        return;
    }

    m_shutdown = true;

    // Detach libcurl's callbacks *first*. Both curl_multi_remove_handle() and
    // curl_multi_cleanup() call back into us, and TimerCallback would happily
    // arm a fresh timer in the middle of teardown.
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, nullptr);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERFUNCTION,  nullptr);

    ++m_timer_generation;
    m_timer.cancel();

    // Every handle we were given must get exactly one callback, otherwise
    // whoever is waiting on it hangs forever.
    auto transfers = std::move(m_transfers);
    m_transfers.clear();

    for (auto& [ easy, callback ] : transfers)
    {
        curl_multi_remove_handle(m_multi, easy);
        SafeInvoke(std::move(callback), easy, CURLE_ABORTED_BY_CALLBACK);
        curl_easy_cleanup(easy);
    }

    m_sockets.clear();

    curl_multi_cleanup(m_multi);
    m_multi = nullptr;
}

void CurlMulti::AddTransfer(CURL* easy, TransferComplete callback)
{
    if (easy == nullptr)
    {
        return;
    }

    // shared_from_this() (not weak) - the instance must stay alive at least
    // long enough for us to take ownership of the handle.
    boost::asio::dispatch(
        m_strand,
        [self = shared_from_this(), easy, callback = std::move(callback)]() mutable
        {
            self->DoAddTransfer(easy, std::move(callback));
        });
}

void CurlMulti::HttpGet(const std::string& url, HttpCallback callback)
{
    auto ex = boost::asio::get_associated_executor(
        callback,
        m_io.get_executor());

    CURL* easy = curl_easy_init();

    if (easy == nullptr)
    {
        // AddTransfer() would silently drop a null handle and the caller would
        // wait forever, so report the failure here.
        BOOST_LOG_TRIVIAL(error) << "CurlMulti: curl_easy_init failed";

        CompleteHttp(m_io, ex, std::move(callback), 0, std::string{});

        return;
    }

    if (auto slot = boost::asio::get_associated_cancellation_slot(callback); slot.is_connected())
    {
        slot.assign([self = shared_from_this(), easy](boost::asio::cancellation_type)
        {
            self->CancelTransfer(easy);
        });
    }

    static const std::string user_agent =
        "porla/" + std::string(porla::BuildInfo::Version());

    auto body = std::make_shared<std::stringstream>();

    curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION,      1L);
    curl_easy_setopt(easy, CURLOPT_MAXREDIRS,           10L);
    curl_easy_setopt(easy, CURLOPT_NOSIGNAL,            1L);
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT,      15L);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT,             300L);
    curl_easy_setopt(easy, CURLOPT_PROTOCOLS_STR,       "http,https");
    curl_easy_setopt(easy, CURLOPT_REDIR_PROTOCOLS_STR, "http,https");
    curl_easy_setopt(easy, CURLOPT_MAXFILESIZE_LARGE,   static_cast<curl_off_t>(kMaxResponseBytes));
    curl_easy_setopt(easy, CURLOPT_WRITEDATA,           body.get());
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,       HttpWriteCallback);
    curl_easy_setopt(easy, CURLOPT_URL,                 url.c_str());
    curl_easy_setopt(easy, CURLOPT_USERAGENT,           user_agent.c_str());

    BOOST_LOG_TRIVIAL(trace) << "CurlMulti::HttpGet: " << url;

    // Capture m_io by reference, not a weak_ptr to self: during Shutdown() from
    // ~CurlMulti the use count is already zero, lock() would fail, and every
    // caller would hang - the exact thing Shutdown() invokes callbacks to avoid.
    AddTransfer(easy, [&io = m_io, ex, body, callback = std::move(callback)](CURL* e, CURLcode) mutable
    {
        if (auto slot = boost::asio::get_associated_cancellation_slot(callback); slot.is_connected())
        {
            slot.clear();
        }

        long response_code = 0;
        curl_easy_getinfo(e, CURLINFO_RESPONSE_CODE, &response_code);

        CompleteHttp(io, ex, std::move(callback), static_cast<int>(response_code), body->str());
    });
}

void CurlMulti::DoAddTransfer(CURL* easy, TransferComplete callback)
{
    if (m_shutdown)
    {
        SafeInvoke(std::move(callback), easy, CURLE_ABORTED_BY_CALLBACK);
        curl_easy_cleanup(easy);
        return;
    }

    // Probe before emplace: emplace() constructs its node before checking for
    // the key, so a rejected insert would destroy a callback we had already
    // moved from, and the caller would wait forever.
    if (m_transfers.find(easy) != m_transfers.end())
    {
        BOOST_LOG_TRIVIAL(error)
            << "CurlMulti: easy handle " << easy << " added twice - ignoring";

        // No cleanup - the first registration owns this handle.
        SafeInvoke(std::move(callback), easy, CURLE_FAILED_INIT);
        return;
    }

    // Insert before adding, so a throwing insert can never leave a handle
    // running inside the multi with nobody to report to.
    auto it = m_transfers.emplace(easy, std::move(callback)).first;

    CURLMcode rc = curl_multi_add_handle(m_multi, easy);

    if (rc != CURLM_OK)
    {
        BOOST_LOG_TRIVIAL(error)
            << "CurlMulti: curl_multi_add_handle failed: " << curl_multi_strerror(rc);

        auto failed = std::move(it->second);
        m_transfers.erase(it);

        SafeInvoke(std::move(failed), easy, CURLE_FAILED_INIT);
        curl_easy_cleanup(easy);
        return;
    }

    // Documented way to kick a newly added transfer. Recent libcurl also arms a
    // zero timer from add_handle, but that isn't contractual.
    SocketAction(CURL_SOCKET_TIMEOUT, 0);
}

void CurlMulti::CancelTransfer(CURL* easy)
{
    boost::asio::dispatch(
        m_strand,
        [self = shared_from_this(), easy]()
        {
            if (self->m_shutdown)                                return;
            if (self->m_transfers.find(easy) == self->m_transfers.end()) return;

            self->FinishTransfer(easy, CURLE_ABORTED_BY_CALLBACK);
        });
}

int CurlMulti::SocketCallback(CURL*, curl_socket_t sock, int what, void* userp, void*)
{
    static_cast<CurlMulti*>(userp)->OnSocketUpdate(sock, what);

    // Always 0: a non-zero return aborts *every* transfer in the multi.
    return 0;
}

void CurlMulti::OnSocketUpdate(curl_socket_t sock, int what)
{
    if (m_shutdown)
    {
        return;
    }

    if (what == CURL_POLL_REMOVE)
    {
        // Destroying the descriptor cancels its in-flight waits; those handlers
        // look the socket up again, find nothing, and return.
        m_sockets.erase(sock);
        return;
    }

    auto it = m_sockets.find(sock);

    if (it == m_sockets.end())
    {
        // asio's stream_descriptor takes ownership of the fd and closes it on
        // destruction, so give it a dup() - libcurl closes the original itself.
        // (The dup shares file status flags with libcurl's socket, so don't go
        // changing O_NONBLOCK etc. on it.)
        const int fd = ::dup(sock);

        if (fd == -1)
        {
            BOOST_LOG_TRIVIAL(error)
                << "CurlMulti: dup() failed for socket " << sock
                << ": " << std::strerror(errno);
            return;
        }

        try
        {
            it = m_sockets.emplace(sock, std::make_unique<SocketState>(m_strand, fd)).first;
        }
        catch (const std::exception& e)
        {
            ::close(fd);

            BOOST_LOG_TRIVIAL(error)
                << "CurlMulti: failed to register socket " << sock << ": " << e.what();
            return;
        }
    }

    it->second->action = what;

    ArmSocket(sock);
}

void CurlMulti::ArmSocket(curl_socket_t sock)
{
    auto it = m_sockets.find(sock);

    if (it == m_sockets.end())
    {
        return;
    }

    auto& state = *it->second;

    using Descriptor = boost::asio::posix::stream_descriptor;

    // Only arm a direction that isn't already armed. Without this check, the
    // re-arm after socket_action() and the arm triggered from libcurl's socket
    // callback stack up, and pending waits grow without bound.
    auto arm = [&](int poll_bit, bool& in_flight, Descriptor::wait_type type)
    {
        if (!(state.action & poll_bit) || in_flight)
        {
            return;
        }

        in_flight = true;

        state.descriptor.async_wait(
            type,
            [weak = weak_from_this(), sock, poll_bit](const boost::system::error_code& ec)
            {
                if (auto self = weak.lock())
                {
                    self->OnSocketReady(sock, ec, poll_bit);
                }
            });
    };

    arm(CURL_POLL_IN,  state.reading, Descriptor::wait_read);
    arm(CURL_POLL_OUT, state.writing, Descriptor::wait_write);
}

bool CurlMulti::StillReadable(curl_socket_t sock)
{
    auto it = m_sockets.find(sock);

    if (it == m_sockets.end() || !(it->second->action & CURL_POLL_IN))
    {
        return false;
    }

    pollfd pfd{ it->second->descriptor.native_handle(), POLLIN, 0 };

    // POLLIN only. POLLHUP and POLLERR latch permanently on a dead fd, and
    // libcurl can keep a socket registered after parking the connection in its
    // pool - we'd spin at 100% CPU forever. EOF sets POLLIN anyway.
    return ::poll(&pfd, 1, 0) == 1 && (pfd.revents & POLLIN);
}

void CurlMulti::DriveSocket(curl_socket_t sock, int mask)
{
    SocketAction(sock, mask);

    // asio's epoll reactor is edge-triggered, but libcurl does not necessarily
    // drain the socket before returning, so no further event would ever arrive.
    // Re-drive ourselves - via post(), so one busy socket can't starve the
    // io_context, and without touching the in-flight flags, since no wait
    // completed here.
    if ((mask & CURL_CSELECT_IN) && m_running > 0 && StillReadable(sock))
    {
        boost::asio::post(
            m_strand,
            [weak = weak_from_this(), sock]()
            {
                if (auto self = weak.lock(); self && !self->m_shutdown)
                {
                    self->DriveSocket(sock, CURL_CSELECT_IN);
                }
            });

        // The re-drive re-arms once the socket runs dry.
        return;
    }

    // Re-arm using the mask libcurl wants *now* (socket_action above may have
    // changed it via the socket callback).
    ArmSocket(sock);
}

void CurlMulti::OnSocketReady(curl_socket_t sock, const boost::system::error_code& ec, int direction)
{
    if (m_shutdown)
    {
        return;
    }

    auto it = m_sockets.find(sock);

    if (it == m_sockets.end())
    {
        // libcurl removed the socket while this handler was queued.
        return;
    }

    bool& in_flight = (direction == CURL_POLL_IN)
        ? it->second->reading
        : it->second->writing;

    in_flight = false;

    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    if (ec)
    {
        BOOST_LOG_TRIVIAL(debug)
            << "CurlMulti: wait failed on socket " << sock << ": " << ec.message();

        SocketAction(sock, CURL_CSELECT_ERR);

        return;
    }

    DriveSocket(sock, direction == CURL_POLL_IN ? CURL_CSELECT_IN : CURL_CSELECT_OUT);
}

int CurlMulti::TimerCallback(CURLM*, long timeout_ms, void* userp)
{
    auto* self = static_cast<CurlMulti*>(userp);

    if (self->m_shutdown)
    {
        return 0;
    }

    const std::uint64_t generation = ++self->m_timer_generation;

    self->m_timer.cancel();

    if (timeout_ms < 0)
    {
        // libcurl no longer needs a timeout.
        return 0;
    }

    // timeout_ms == 0 means "call socket_action as soon as possible". A
    // zero-length timer gets us there on the next io_context iteration and,
    // unlike a bare post(), remains cancellable.
    self->m_timer.expires_after(std::chrono::milliseconds(timeout_ms));

    self->m_timer.async_wait(
        [weak = self->weak_from_this(), generation](const boost::system::error_code& ec)
        {
            if (ec)
            {
                return;
            }

            if (auto self = weak.lock())
            {
                self->OnTimeout(generation);
            }
        });

    return 0;
}

void CurlMulti::OnTimeout(std::uint64_t generation)
{
    if (m_shutdown)
    {
        return;
    }

    // A timer that expires just before cancel() still delivers a success
    // error_code, so discriminate on the generation instead.
    if (generation != m_timer_generation)
    {
        return;
    }

    SocketAction(CURL_SOCKET_TIMEOUT, 0);
}

void CurlMulti::SocketAction(curl_socket_t sock, int event_bitmask)
{
    CURLMcode rc = curl_multi_socket_action(m_multi, sock, event_bitmask, &m_running);

    if (rc != CURLM_OK)
    {
        BOOST_LOG_TRIVIAL(error)
            << "CurlMulti: curl_multi_socket_action failed: " << curl_multi_strerror(rc);
    }

    CheckCompleted();
}

void CurlMulti::CheckCompleted()
{
    if (m_in_check_completed)
    {
        return;
    }

    m_in_check_completed = true;

    struct Guard
    {
        bool& flag;
        ~Guard() { flag = false; }
    } guard{ m_in_check_completed };

    CURLMsg* msg = nullptr;
    int      pending = 0;

    while ((msg = curl_multi_info_read(m_multi, &pending)) != nullptr)
    {
        if (msg->msg != CURLMSG_DONE)
        {
            continue;
        }

        // Copy out immediately: `msg` is invalidated by the next libcurl call
        // on this multi handle, and FinishTransfer() makes several.
        CURL* const    easy   = msg->easy_handle;
        const CURLcode result = msg->data.result;

        FinishTransfer(easy, result);
    }
}

void CurlMulti::FinishTransfer(CURL* easy, CURLcode result)
{
    curl_multi_remove_handle(m_multi, easy);

    TransferComplete callback;

    if (auto it = m_transfers.find(easy); it != m_transfers.end())
    {
        // Erase *before* invoking: the callback may call AddTransfer(), and an
        // insert that rehashes would invalidate `it`.
        callback = std::move(it->second);
        m_transfers.erase(it);
    }

    SafeInvoke(std::move(callback), easy, result);

    curl_easy_cleanup(easy);
}

void CurlMulti::SafeInvoke(TransferComplete&& callback, CURL* easy, CURLcode result)
{
    if (!callback)
    {
        return;
    }

    try
    {
        std::move(callback)(easy, result);
    }
    catch (const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "CurlMulti: transfer callback threw: " << e.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "CurlMulti: transfer callback threw an unknown exception";
    }
}
