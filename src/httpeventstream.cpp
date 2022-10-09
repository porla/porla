#include "httpeventstream.hpp"

#include <queue>

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

#include "json/all.hpp"
#include "session.hpp"

namespace lt = libtorrent;

using json = nlohmann::json;
using porla::HttpEventStream;

class HttpEventStream::ContextState : public std::enable_shared_from_this<HttpEventStream::ContextState>
{
public:
    explicit ContextState(std::shared_ptr<porla::HttpContext> context)
        : m_ctx(std::move(context))
    {
    }

    bool IsDead() const { return m_dead; }

    void QueueWrite(std::string data)
    {
        if (m_dead) { return; }
        m_sendData.push(std::move(data));
        MaybeWrite();
    }

private:
    void MaybeWrite()
    {
        if (m_dead || m_isWriting || m_sendData.empty())
        {
            return;
        }

        const std::string& data = m_sendData.front();

        boost::asio::async_write(
            m_ctx->Stream(),
            boost::asio::buffer(data),
            [_this = shared_from_this()](boost::system::error_code ec, int b)
            {
                _this->OnWrite(ec, b);
            });

        m_isWriting = true;
    }

    void OnWrite(boost::system::error_code ec, int bytes)
    {
        m_isWriting = false;
        m_sendData.pop();

        if (ec)
        {
            m_dead = true;

            if (ec == boost::asio::error::broken_pipe
                || ec == boost::asio::error::timed_out)
            {
                // Client disconnected
                return;
            }

            BOOST_LOG_TRIVIAL(error) << "Failed to write " << bytes << " bytes: " << ec.message();

            return;
        }

        m_sent += bytes;

        MaybeWrite();
    }

    bool m_dead {false};
    bool m_isWriting {false};
    int64_t m_sent{0};
    std::queue<std::string> m_sendData;
    std::shared_ptr<porla::HttpContext> m_ctx;
};

HttpEventStream::HttpEventStream(porla::ISession &session)
    : m_session(session)
{
    m_stateUpdateConnection = m_session.OnStateUpdate([this](auto s) { OnStateUpdate(s); });
}

HttpEventStream::HttpEventStream(const HttpEventStream& hes)
    : m_session(hes.m_session)
{
    m_stateUpdateConnection = m_session.OnStateUpdate([this](auto s) { OnStateUpdate(s); });
}

HttpEventStream::~HttpEventStream()
{
    m_stateUpdateConnection.disconnect();
}

void HttpEventStream::operator()(std::shared_ptr<HttpContext> context)
{
    std::string headers = "HTTP/1.1 200 OK\n"
                          "Connection: keep-alive\n"
                          "Content-Type: text/event-stream\n"
                          "Cache-Control: no-cache, no-transform\n\n";

    json::array_t torrents;

    m_session.ForEach(
        [&torrents](const auto &ts)
        {
            torrents.push_back({
                {"info_hash", ts.info_hashes}
            });
        });

    json initial = {
        {"torrents", torrents}
    };

    std::stringstream evt;
    evt << "event: initial_state\n";
    evt << "data: {}\n\n";

    auto state = std::make_shared<ContextState>(std::move(context));
    state->QueueWrite(headers);
    state->QueueWrite(evt.str());

    m_ctxs.push_back(state);
}

void HttpEventStream::Broadcast(const std::string& name, const std::string& data)
{
    if (m_ctxs.empty())
    {
        return;
    }

    std::stringstream evt;
    evt << "event: "<< name << "\n";
    evt << "data: " << data << "\n\n";

    m_ctxs.erase(
        std::remove_if(
            m_ctxs.begin(),
            m_ctxs.end(),
            [](auto ptr) { return ptr->IsDead(); }),
        m_ctxs.end());

    for (auto& ctx : m_ctxs)
    {
        ctx->QueueWrite(evt.str());
    }
}

void HttpEventStream::OnStateUpdate(const std::vector<lt::torrent_status>& torrents)
{
    json state = json::array();

    for (const auto& status : torrents)
    {
        json j = {
            {"info_hash", status.info_hashes}
        };

        state.push_back(j);
    }

    Broadcast("state_update", state.dump());
}
