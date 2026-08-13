#include "curlmulti.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <unistd.h>

#include <boost/log/trivial.hpp>

#include "buildinfo.hpp"

using porla::CurlMulti;

static size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::stringstream* ss = reinterpret_cast<std::stringstream*>(userdata);
    ss->write(ptr, nmemb);
    return nmemb;
}

struct CurlMulti::SocketState
{
    SocketState(boost::asio::io_context& io, int fd)
        : descriptor(io, fd)
    {
    }

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
        SafeInvoke(callback, easy, CURLE_ABORTED_BY_CALLBACK);
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
    std::stringstream user_agent;
    user_agent << "porla/" << porla::BuildInfo::Version();

    auto body = std::make_shared<std::stringstream>();

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, body.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWriteCallback);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.str().c_str());

    BOOST_LOG_TRIVIAL(trace) << "WebUI::HttpGet: " << url.c_str();

    AddTransfer(curl, [w = weak_from_this(), body, callback](CURL* easy, CURLcode result)
    {
        auto self = w.lock();

        if (!self)
        {
            return;
        }

        long response_code;
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code);

        boost::asio::post(self->m_io, [callback, body, response_code]()
        {
            callback(response_code, body->str());
        });
    });
}

void CurlMulti::DoAddTransfer(CURL* easy, TransferComplete callback)
{
    if (m_shutdown)
    {
        SafeInvoke(callback, easy, CURLE_ABORTED_BY_CALLBACK);
        curl_easy_cleanup(easy);
        return;
    }

    // Insert before adding, so a throwing insert can never leave a handle
    // running inside the multi with nobody to report to.
    auto [ it, inserted ] = m_transfers.emplace(easy, std::move(callback));

    if (!inserted)
    {
        BOOST_LOG_TRIVIAL(error)
            << "CurlMulti: easy handle " << easy << " added twice - ignoring";
        return;
    }

    CURLMcode rc = curl_multi_add_handle(m_multi, easy);

    if (rc != CURLM_OK)
    {
        BOOST_LOG_TRIVIAL(error)
            << "CurlMulti: curl_multi_add_handle failed: " << curl_multi_strerror(rc);

        auto failed = std::move(it->second);
        m_transfers.erase(it);

        SafeInvoke(failed, easy, CURLE_FAILED_INIT);
        curl_easy_cleanup(easy);
        return;
    }

    // curl_multi_add_handle() sets a zero timeout, which drives TimerCallback
    // and kicks the transfer off - no explicit socket_action needed here.
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
            it = m_sockets.emplace(sock, std::make_unique<SocketState>(m_io, fd)).first;
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

    // Only arm a direction that isn't already armed. Without this check, the
    // re-arm after socket_action() and the arm triggered from libcurl's socket
    // callback stack up, and pending waits grow without bound.
    if ((state.action & CURL_POLL_IN) && !state.reading)
    {
        state.reading = true;

        state.descriptor.async_wait(
            boost::asio::posix::stream_descriptor::wait_read,
            boost::asio::bind_executor(
                m_strand,
                [weak = weak_from_this(), sock](const boost::system::error_code& ec)
                {
                    if (auto self = weak.lock())
                    {
                        self->OnSocketReady(sock, ec, CURL_POLL_IN);
                    }
                }));
    }

    if ((state.action & CURL_POLL_OUT) && !state.writing)
    {
        state.writing = true;

        state.descriptor.async_wait(
            boost::asio::posix::stream_descriptor::wait_write,
            boost::asio::bind_executor(
                m_strand,
                [weak = weak_from_this(), sock](const boost::system::error_code& ec)
                {
                    if (auto self = weak.lock())
                    {
                        self->OnSocketReady(sock, ec, CURL_POLL_OUT);
                    }
                }));
    }
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

    // Clear the in-flight flag *before* calling into libcurl, so a re-arm
    // triggered from within the socket callback isn't suppressed.
    if (direction == CURL_POLL_IN) { it->second->reading = false; }
    else                           { it->second->writing = false; }

    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    const int mask = ec
        ? CURL_CSELECT_ERR
        : (direction == CURL_POLL_IN ? CURL_CSELECT_IN : CURL_CSELECT_OUT);

    SocketAction(sock, mask);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(debug)
            << "CurlMulti: wait failed on socket " << sock << ": " << ec.message();

        // Don't re-arm after a hard error - we'd just spin on a permanently
        // signalling fd. libcurl will fail the transfer and remove the socket.
        return;
    }

    // Re-arm using the mask libcurl wants *now* (socket_action above may have
    // changed it via the socket callback).
    ArmSocket(sock);
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

    if (m_running == 0)
    {
        ++m_timer_generation;
        m_timer.cancel();
    }
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

    SafeInvoke(callback, easy, result);

    curl_easy_cleanup(easy);
}

void CurlMulti::SafeInvoke(const TransferComplete& callback, CURL* easy, CURLcode result)
{
    if (!callback)
    {
        return;
    }

    try
    {
        callback(easy, result);
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
