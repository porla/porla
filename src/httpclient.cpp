#include "httpclient.hpp"

#include <boost/asio/ssl/context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>

#include "uri.hpp"

using porla::HttpClient;

struct HttpClient::RequestState : public std::enable_shared_from_this<HttpClient::RequestState>
{
    explicit RequestState(boost::asio::io_context& io)
        : m_resolver(io)
        , m_stream(io)
        , m_ssl_ctx(boost::asio::ssl::context::tls_client)
        , m_ssl_stream(io, m_ssl_ctx)
    {
        m_ssl_ctx.set_verify_mode(
            boost::asio::ssl::verify_peer
            | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        m_ssl_ctx.set_default_verify_paths();
    }

    void Send()
    {
        m_resolver.async_resolve(
            m_uri.host,
            std::to_string(m_uri.port),
            boost::beast::bind_front_handler(
                [_this = shared_from_this()](auto && PH1, auto && PH2) { _this->OnAsyncResolve(PH1, PH2); }));
    }

    void OnAsyncResolve(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type &results)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "HttpClient resolve error: " << ec.message();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "HttpClient resolved hosts";

        auto& stream = m_uri.scheme == "https"
            ? boost::beast::get_lowest_layer(m_ssl_stream)
            : m_stream;

        stream.async_connect(
            results,
            boost::beast::bind_front_handler(
                [_this = shared_from_this()](auto && PH1, auto && PH2) { _this->OnAsyncConnect(PH1, PH2); }));
    }

    void OnAsyncConnect(
        boost::system::error_code ec,
        const boost::asio::ip::tcp::resolver::results_type::endpoint_type& type)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "HttpClient connect error: " << ec.message();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "HttpClient connected to " << type.address().to_string();

        if (m_uri.scheme == "https")
        {
            if (!SSL_set_tlsext_host_name(m_ssl_stream.native_handle(), m_uri.host.c_str()))
            {
                ec = {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                BOOST_LOG_TRIVIAL(error) << "Failed to set tlsext hostname: " << ec.message();
                return;
            }

            BOOST_LOG_TRIVIAL(debug) << "HttpClient starting SSL handshake";

            m_ssl_stream.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(
                    [_this = shared_from_this()](auto && PH1) { _this->OnAsyncHandshake(PH1); }));
        }
        else
        {
            SendRequest();
        }
    }

    void OnAsyncHandshake(boost::system::error_code ec)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Webhook SSL handshake error: " << ec.message();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "HttpClient SSL handshake complete";

        SendRequest();
    }

    void SendRequest()
    {
        auto req = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
        req->method(m_payload.empty() ? boost::beast::http::verb::get : boost::beast::http::verb::post);
        req->target(m_uri.path);
        req->version(11);

        req->set(boost::beast::http::field::content_type, "application/json");

        // Set these headers after user-specified headers. These cannot be overridden.

        req->set(boost::beast::http::field::host, m_uri.host + ":" + std::to_string(m_uri.port));
        req->set(boost::beast::http::field::user_agent, "porla/1.0");
        req->body() = m_payload;
        req->prepare_payload();

        BOOST_LOG_TRIVIAL(debug) << "HttpClient writing request";
        BOOST_LOG_TRIVIAL(debug) << *req;

        if (m_uri.scheme == "https")
        {
            boost::beast::http::async_write(
                m_ssl_stream,
                *req,
                boost::beast::bind_front_handler(
                    [_this = shared_from_this(), req](auto &&PH1, auto &&PH2) { _this->OnAsyncWrite(PH1, PH2); }));
        }
        else
        {
            boost::beast::http::async_write(
                m_stream,
                *req,
                boost::beast::bind_front_handler(
                    [_this = shared_from_this(), req](auto &&PH1, auto &&PH2) { _this->OnAsyncWrite(PH1, PH2); }));
        }
    }

    void OnAsyncWrite(boost::system::error_code ec, std::size_t size)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "HttpClient write error: " << ec.message();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "HttpClient request written (" << size << " bytes)";

        if (m_uri.scheme == "https")
        {
            boost::beast::http::async_read(
                m_ssl_stream,
                m_buffer,
                m_res,
                boost::beast::bind_front_handler(
                    [_this = shared_from_this()](auto &&PH1, auto &&PH2)
                    {
                        _this->OnAsyncRead(
                            std::forward<decltype(PH1)>(PH1),
                            std::forward<decltype(PH2)>(PH2));
                    }));
        }
        else
        {
            boost::beast::http::async_read(
                m_stream,
                m_buffer,
                m_res,
                boost::beast::bind_front_handler(
                    [_this = shared_from_this()](auto && PH1, auto && PH2)
                    {
                        _this->OnAsyncRead(
                            std::forward<decltype(PH1)>(PH1),
                            std::forward<decltype(PH2)>(PH2));
                    }));
        }
    }

    void OnAsyncRead(boost::system::error_code ec, std::size_t size)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(info) << "Webhook read error: " << ec.message();
            return;
        }

        BOOST_LOG_TRIVIAL(debug) << "HttpClient read " << size << " bytes";
        BOOST_LOG_TRIVIAL(debug) << m_res;

        m_callback();
    }

    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::flat_buffer m_buffer;
    boost::beast::http::response<boost::beast::http::string_body> m_res;
    boost::beast::tcp_stream m_stream;
    boost::asio::ssl::context m_ssl_ctx;
    boost::beast::ssl_stream<boost::beast::tcp_stream> m_ssl_stream;
    porla::Uri m_uri;
    std::string m_payload;
    std::function<void()> m_callback;
};

HttpClient::HttpClient(boost::asio::io_context& io)
    : m_io(io)
{
}

void HttpClient::SendAsync(const Request& req, const std::function<void()>& callback)
{
    auto state = std::make_shared<RequestState>(m_io);
    state->m_callback = callback;
    state->m_payload  = req.body;

    if (!Uri::Parse(req.url, state->m_uri))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to parse uri: " << req.url;
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Sending HTTP request to " << req.url;

    state->Send();
}
