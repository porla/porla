#include "ntfy.hpp"

#include <memory>

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/log/trivial.hpp>

using porla::Workflows::ActionCallback;
using porla::Workflows::Actions::Push::Ntfy;

struct NtfyRequest;

void OnAsyncResolve(
    const boost::system::error_code& ec,
    const boost::asio::ip::tcp::resolver::results_type& results,
    const std::shared_ptr<NtfyRequest>& req);

void OnAsyncConnect(
    const boost::system::error_code& ec,
    const boost::asio::ip::tcp::resolver::results_type::endpoint_type& type,
    const std::shared_ptr<NtfyRequest>& req);

void OnAsyncHandshake(
    const boost::system::error_code& ec,
    const std::shared_ptr<NtfyRequest>& req);

void OnAsyncWrite(
    const boost::system::error_code& ec,
    std::size_t size,
    const std::shared_ptr<NtfyRequest>& req);

void OnAsyncRead(
    const boost::system::error_code& ec,
    std::size_t size,
    const std::shared_ptr<NtfyRequest>& req);

struct NtfyRequest
{
    explicit NtfyRequest(boost::asio::io_context& io, const std::shared_ptr<ActionCallback>& cb)
        : callback(cb)
        , resolver(io)
        , ssl_ctx(boost::asio::ssl::context::tls_client)
        , stream(io, ssl_ctx)
    {
        ssl_ctx.set_verify_mode(
            boost::asio::ssl::verify_peer
            | boost::asio::ssl::context::verify_fail_if_no_peer_cert);

        ssl_ctx.set_default_verify_paths();
    }

    boost::beast::flat_buffer                                     buffer;
    std::shared_ptr<ActionCallback>                               callback;
    boost::asio::ip::tcp::resolver                                resolver;
    boost::beast::http::response<boost::beast::http::string_body> response;
    boost::asio::ssl::context                                     ssl_ctx;
    boost::beast::ssl_stream<boost::beast::tcp_stream>            stream;
};

Ntfy::Ntfy(boost::asio::io_context& io)
    : m_io(io)
{
}

void Ntfy::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    auto req = std::make_shared<NtfyRequest>(m_io, callback);
    req->resolver.async_resolve(
        "ntfy.sh",
        "https",
        boost::beast::bind_front_handler(
            [req]( auto && PH1, auto && PH2)
            {
                OnAsyncResolve(PH1, PH2, req);
            }));
}

void OnAsyncResolve(
    const boost::system::error_code& ec,
    const boost::asio::ip::tcp::resolver::results_type& results,
    const std::shared_ptr<NtfyRequest>& req)
{
    boost::beast::get_lowest_layer(req->stream)
        .async_connect(
            results,
            boost::beast::bind_front_handler(
                [req](auto && PH1, auto && PH2)
                {
                    OnAsyncConnect(PH1, PH2, req);
                }));
}

void OnAsyncConnect(
    const boost::system::error_code& ec,
    const boost::asio::ip::tcp::resolver::results_type::endpoint_type& type,
    const std::shared_ptr<NtfyRequest>& req)
{
    if (!SSL_set_tlsext_host_name(req->stream.native_handle(), "ntfy.sh"))
    {
        return;
    }

    req->stream.async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::beast::bind_front_handler(
            [req](auto && PH1)
            {
                OnAsyncHandshake(PH1, req);
            }));
}

void OnAsyncHandshake(
    const boost::system::error_code& ec,
    const std::shared_ptr<NtfyRequest>& req)
{
    auto http_request = std::make_shared<boost::beast::http::request<boost::beast::http::string_body>>();
    http_request->method(boost::beast::http::verb::post);
    http_request->target("/porla-secret-topic");
    http_request->version(11);

    http_request->set(boost::beast::http::field::content_type, "application/json");

    // Set these headers after user-specified headers. These cannot be overridden.

    http_request->set(boost::beast::http::field::host, "ntfy.sh");
    http_request->set(boost::beast::http::field::user_agent, "porla/1.0");
    http_request->body() = "Hello!";
    http_request->prepare_payload();

    boost::beast::http::async_write(
        req->stream,
        *http_request,
        boost::beast::bind_front_handler(
            [http_request, req](auto &&PH1, auto &&PH2)
            {
                OnAsyncWrite(PH1, PH2, req);
            }));
}

void OnAsyncWrite(
    const boost::system::error_code& ec,
    std::size_t size,
    const std::shared_ptr<NtfyRequest>& req)
{
    boost::beast::http::async_read(
        req->stream,
        req->buffer,
        req->response,
        boost::beast::bind_front_handler(
            [req](auto && PH1, auto && PH2)
            {
                OnAsyncRead(
                    std::forward<decltype(PH1)>(PH1),
                    std::forward<decltype(PH2)>(PH2),
                    req);
            }));
}

void OnAsyncRead(
    const boost::system::error_code& ec,
    std::size_t size,
    const std::shared_ptr<NtfyRequest>& req)
{
    BOOST_LOG_TRIVIAL(info) << req->response;

    req->callback->Complete({});
}
