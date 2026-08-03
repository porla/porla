#include "curlmulti.hpp"

#include <boost/log/trivial.hpp>

using porla::CurlMulti;

CurlMulti::CurlMulti(boost::asio::io_context& io)
    : m_io(io)
    , m_multi(curl_multi_init())
    , m_timer(io)
{
    if (m_multi == nullptr)
    {
        throw std::runtime_error("curl_multi_init failed");
    }

    curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, &CurlMulti::SocketCallback);
    curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERFUNCTION, &CurlMulti::TimerCallback);
    curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA, this);
}

CurlMulti::~CurlMulti()
{
    m_timer.cancel();

    for (auto& [ easy, _ ] : m_transfers)
    {
        curl_multi_remove_handle(m_multi, easy);
        curl_easy_cleanup(easy);
    }

    m_sockets.clear();
    m_transfers.clear();

    curl_multi_cleanup(m_multi);
}

void CurlMulti::AddTransfer(CURL* easy, TransferComplete callback)
{
    CURLMcode rc = curl_multi_add_handle(m_multi, easy);

    if (rc != CURLM_OK)
    {
        curl_easy_cleanup(easy);

        throw std::runtime_error(
            std::string("curl_multi_add_handle failed: ") + curl_multi_strerror(rc));
    }

    m_transfers.insert({ easy, callback });
}

int CurlMulti::SocketCallback(CURL* easy, curl_socket_t sock, int action, void* userp, void* socketp)
{
    auto* self = static_cast<CurlMulti*>(userp);

    if (action == CURL_POLL_REMOVE)
    {
        self->m_sockets.erase(sock);
        return 0;
    }

    if (self->m_sockets.find(sock) == self->m_sockets.end())
    {
        int fd = dup(sock);

        if (fd == -1)
        {
            BOOST_LOG_TRIVIAL(error) << "CurlMulti: dup() failed for socket " << sock;
            return -1;
        }

        self->m_sockets[sock] = std::make_unique<boost::asio::posix::stream_descriptor>(
            self->m_io, fd);
    }

    self->WatchSocket(sock, action);

    return 0;
}

int CurlMulti::TimerCallback(CURLM* multi, long timeout_ms, void* userp)
{
    auto* self = static_cast<CurlMulti*>(userp);

    self->m_timer.cancel();

    if (timeout_ms < 0)
    {
        return 0;
    }

    if (timeout_ms == 0)
    {
        // Curl wants us to call socket_action immediately
        boost::asio::post(
            self->m_io,
            [self]()
            {
                int running = 0;
                curl_multi_socket_action(self->m_multi, CURL_SOCKET_TIMEOUT, 0, &running);

                self->CheckCompleted();
            });

        return 0;
    }

    self->m_timer.expires_after(std::chrono::milliseconds(timeout_ms));

    self->m_timer.async_wait([self](boost::system::error_code ec)
    {
        self->OnTimeout(ec);
    });

    return 0;
}

void CurlMulti::WatchSocket(curl_socket_t sock, int action)
{
    auto it = m_sockets.find(sock);

    if (it == m_sockets.end())
    {
        return;
    }

    auto& stream_descriptor = it->second;

    if (action & CURL_POLL_IN)
    {
        stream_descriptor->async_wait(
            boost::asio::posix::stream_descriptor::wait_read,
            [this, sock](boost::system::error_code ec)
            {
                OnSocketReady(sock, ec, CURL_CSELECT_IN);
            });
    }

    if (action & CURL_POLL_OUT)
    {
        stream_descriptor->async_wait(
            boost::asio::posix::stream_descriptor::wait_write,
            [this, sock](boost::system::error_code ec)
            {
                OnSocketReady(sock, ec, CURL_CSELECT_OUT);
            });
    }
}

void CurlMulti::OnSocketReady(curl_socket_t sock, boost::system::error_code ec, int action)
{
    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }

    int curl_action = action;

    if (ec)
    {
        curl_action = CURL_CSELECT_ERR;
    }

    int running = 0;
    curl_multi_socket_action(m_multi, sock, action, &running);

    CheckCompleted();

    if (!ec)
    {
        // Re-watch the socket if it still exists
        auto it = m_sockets.find(sock);

        if (it != m_sockets.end())
        {
            WatchSocket(
                sock,
                action == CURL_CSELECT_IN
                    ? CURL_POLL_IN
                    : CURL_POLL_OUT);
        }
    }
}

void CurlMulti::OnTimeout(boost::system::error_code ec)
{
    if (ec)
    {
        return;
    }

    int running = 0;
    curl_multi_socket_action(m_multi, CURL_SOCKET_TIMEOUT, 0, &running);

    CheckCompleted();
}

void CurlMulti::CheckCompleted()
{
    CURLMsg* msg;
    int pending;

    while ((msg = curl_multi_info_read(m_multi, &pending)))
    {
        if (msg->msg != CURLMSG_DONE)
        {
            continue;
        }

        auto it = m_transfers.find(msg->easy_handle);

        if (it != m_transfers.end())
        {
            try
            {
                it->second(msg->easy_handle);
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error)
                    << "CurlMulti: transfer callback threw: " << e.what();
            }

            m_transfers.erase(it);
        }

        curl_multi_remove_handle(m_multi, msg->easy_handle);
        curl_easy_cleanup(msg->easy_handle);
    }
}