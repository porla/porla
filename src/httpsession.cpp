#include "httpsession.hpp"

#include <filesystem>
#include <memory>
#include <utility>

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>
#include <uriparser/Uri.h>

#include "httpcontext.hpp"
#include "httpmiddleware.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;
using BasicHttpRequest = boost::beast::http::request<boost::beast::http::string_body>;

using porla::HttpSession;

class HttpSession::MiddlewareContext : public porla::HttpContext
{
public:
    explicit MiddlewareContext(
        std::shared_ptr<HttpSession> session,
        BasicHttpRequest req,
        std::vector<porla::HttpMiddleware> mws,
        std::vector<porla::HttpMiddleware>::const_iterator current)
        : m_session(std::move(session))
        , m_req(std::move(req))
        , m_mws(std::move(mws))
        , m_curr(current)
    {
        UriUriA uri = {};

        UriParserStateA state;
        state.uri = &uri;

        std::string faked_url = "http://porla" + m_req.target().to_string();

        if (uriParseUriA(&state, faked_url.c_str()) != URI_SUCCESS)
        {
            BOOST_LOG_TRIVIAL(error) << "Invalid uri: " << m_req.target();
        }
        else
        {
            UriPathSegmentStructA *head(uri.pathHead);
            std::stringstream accum;

            const auto fromRange = [](const UriTextRangeA &range) -> std::string
            {
                return {range.first, range.afterLast};
            };

            while (head)
            {
                accum << "/" << fromRange(head->text);
                head = head->next;
            }

            m_uri = Uri{
                .path = accum.str()
            };
        }
    }

    void Next() override
    {
        auto next = m_curr + 1;
        auto ctx = std::make_shared<MiddlewareContext>(
            m_session,
            m_req,
            m_mws,
            next);

        (*next)(ctx);
    }

    boost::beast::http::request<boost::beast::http::string_body>& Request() override
    {
        return m_req;
    }

    boost::beast::tcp_stream& Stream() override
    {
        return m_session->m_stream;
    }

    porla::HttpContext::Uri& RequestUri() override
    {
        return m_uri;
    }

    void Write(std::string body) override
    {
        namespace http = boost::beast::http;

        http::response<http::string_body> res{http::status::ok, m_req.version()};
        res.set(http::field::server, "porla/1.0");
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(m_req.keep_alive());
        res.body() = body;
        res.prepare_payload();

        m_session->m_queue(std::move(res));
    }

    void Write(boost::beast::http::response<boost::beast::http::file_body> res) override
    {
        m_session->m_queue(std::move(res));
    }

    void Write(boost::beast::http::response<boost::beast::http::string_body> res) override
    {
        m_session->m_queue(std::move(res));
    }

    void WriteJson(const nlohmann::json& j) override
    {
        namespace http = boost::beast::http;

        http::response<http::string_body> res{http::status::ok, m_req.version()};
        res.set(http::field::server, "porla/1.0");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(m_req.keep_alive());
        res.body() = j.dump();
        res.prepare_payload();

        m_session->m_queue(std::move(res));
    }

private:
    std::shared_ptr<HttpSession> m_session;
    BasicHttpRequest m_req;
    std::vector<porla::HttpMiddleware> m_mws;
    std::vector<porla::HttpMiddleware>::const_iterator m_curr;
    Uri m_uri;
};

HttpSession::HttpSession(
    boost::asio::ip::tcp::socket&& socket,
    std::vector<porla::HttpMiddleware> middlewares)
    : m_stream(std::move(socket))
    , m_queue(*this)
    , m_middlewares(std::move(middlewares))
{
}

void HttpSession::Run()
{
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    boost::asio::dispatch(
        m_stream.get_executor(),
        boost::beast::bind_front_handler(
            &HttpSession::BeginRead,
            shared_from_this()));
}

void HttpSession::Stop()
{
    m_stream.close();
}

void HttpSession::BeginRead()
{
    m_parser.emplace();
    m_parser->body_limit(10000000);

    m_stream.expires_after(std::chrono::seconds(30));

    // Read a request using the parser-oriented interface
    boost::beast::http::async_read(
        m_stream,
        m_buffer,
        *m_parser,
        boost::beast::bind_front_handler(
            &HttpSession::EndRead,
            shared_from_this()));
}

void HttpSession::EndRead(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    namespace http = boost::beast::http;

    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
    {
        BOOST_LOG_TRIVIAL(info) << "Stream closing";
        return BeginClose();
    }

    if(ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error when reading HTTP request: " << ec.message();
        return;
    }

    // Check for matching handler
    auto req = m_parser->release();

    if (!m_middlewares.empty())
    {
        auto first = m_middlewares.begin();
        auto ctx = std::make_shared<MiddlewareContext>(
            shared_from_this(),
            std::move(req),
            m_middlewares,
            first);

        (*first)(ctx);

        return;
    }

    auto const not_found = [&req](boost::beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    m_queue(not_found(req.target()));

    // If we aren't at the queue limit, try to pipeline another request
    if(!m_queue.IsFull())
    {
        BeginRead();
    }
}

void HttpSession::EndWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error when writing data: " << ec.message();
        return;
    }

    if (close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return BeginClose();
    }

    // Inform the queue that a write completed
    if (m_queue.OnWrite())
    {
        // Read another request
        BeginRead();
    }
}

void HttpSession::BeginClose()
{
    // Send a TCP shutdown
    boost::beast::error_code ec;
    m_stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}