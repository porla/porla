#include "webhookclient.hpp"

#include <boost/asio/ssl/context.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>

#include "session.hpp"

using porla::WebhookClient;

struct HttpClient : public std::enable_shared_from_this<HttpClient>
{
    explicit HttpClient(boost::asio::io_context& io)
        : m_io(io)
        , m_resolver(io)
        , m_ctx(boost::asio::ssl::context::tlsv12_client)
        , m_stream(io, m_ctx)
    {
        m_ctx.set_verify_mode(boost::asio::ssl::verify_peer);

        req_.version(11);
        req_.method(boost::beast::http::verb::get);
        req_.target("/");
        req_.set(boost::beast::http::field::host, "google.com");
        req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    }

    void Get()
    {
        if(! SSL_set_tlsext_host_name(m_stream.native_handle(), "google.com"))
        {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            BOOST_LOG_TRIVIAL(error) << ec.message();
            return;
        }

        m_resolver.async_resolve(
            "google.com",
            "443",
            boost::beast::bind_front_handler(
                &HttpClient::OnResolve,
                shared_from_this()));
    }

private:
    void OnResolve(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type& results)
    {
        boost::beast::get_lowest_layer(m_stream).async_connect(
            results,
            boost::beast::bind_front_handler(
                &HttpClient::OnConnect,
                shared_from_this()));
    }

    void OnConnect(boost::system::error_code ec, const boost::asio::ip::tcp::resolver::results_type::endpoint_type& type)
    {
        BOOST_LOG_TRIVIAL(info) << "Connected?";

        // Perform the SSL handshake
        m_stream.async_handshake(
            boost::asio::ssl::stream_base::client,
            boost::beast::bind_front_handler(
                &HttpClient::OnHandshake,
                shared_from_this()));
    }

    void OnHandshake(boost::system::error_code ec)
    {
        BOOST_LOG_TRIVIAL(info) << "Handshake?";

        boost::beast::http::async_write(
            m_stream,
            req_,
            boost::beast::bind_front_handler(
                &HttpClient::OnWrite,
                shared_from_this()));
    }

    void OnWrite(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        BOOST_LOG_TRIVIAL(info) << "Write " << bytes_transferred;

        boost::beast::http::async_read(
            m_stream,
            buffer_,
            res_,
            boost::beast::bind_front_handler(
                &HttpClient::OnRead,
                shared_from_this()));
    }

    void OnRead(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        BOOST_LOG_TRIVIAL(info) << bytes_transferred;
        BOOST_LOG_TRIVIAL(info) << res_;
    }

    boost::asio::io_context& m_io;
    boost::asio::ip::tcp::resolver m_resolver;
    boost::asio::ssl::context m_ctx;
    boost::beast::ssl_stream<boost::beast::tcp_stream> m_stream;
    boost::beast::http::request<boost::beast::http::empty_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    boost::beast::flat_buffer buffer_;
};

WebhookClient::WebhookClient(boost::asio::io_context& io, ISession& session)
    : m_io(io)
    , m_session(session)
{
    auto client = std::make_shared<HttpClient>(io);
    client->Get();
}
