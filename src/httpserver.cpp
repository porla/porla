#include "httpserver.hpp"

#include <boost/beast.hpp>
#include <boost/log/trivial.hpp>

#include "httpcontext.hpp"
#include "httpmiddleware.hpp"
#include "httpsession.hpp"

using porla::HttpServer;
using porla::HttpSession;

class HttpServer::State : public std::enable_shared_from_this<HttpServer::State>
{
public:
    State(boost::asio::io_context& io, std::string const& host, uint16_t port)
        : m_io(io),
        m_acceptor(boost::asio::make_strand(m_io))
    {
        boost::system::error_code ec;
        auto addr = boost::asio::ip::make_address(host, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to parse address: " << ec.message() << " - defaulting to 127.0.0.1";
            addr = boost::asio::ip::make_address("127.0.0.1");
        }

        auto endpoint = boost::asio::ip::tcp::endpoint{ addr, port };

        m_acceptor.open(endpoint.protocol(), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to open TCP endpoint: " << ec; }

        m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set reuse_address(true): " << ec; }

        m_acceptor.bind(endpoint, ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to bind TCP endpoint: " << ec; }

        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to listen: " << ec; }

        BOOST_LOG_TRIVIAL(info) << "Running HTTP server on " << m_acceptor.local_endpoint();
    }

    auto Endpoint() { return m_acceptor.local_endpoint(); }

    void Start()
    {
        boost::asio::dispatch(
            m_acceptor.get_executor(),
            boost::beast::bind_front_handler(
                &State::BeginAccept,
                shared_from_this()));
    }

    void Stop()
    {
        m_acceptor.cancel();
        m_middlewares.clear();

        for (auto&& s : m_sessions)
        {
            if (auto sl = s.lock())
            {
                sl->Stop();
            }
        }

        m_sessions.clear();
    }

    void Use(const porla::HttpMiddleware& middleware)
    {
        m_middlewares.emplace_back(middleware);
    }

private:
    void BeginAccept()
    {
        m_acceptor.async_accept(
            boost::asio::make_strand(m_io),
            boost::beast::bind_front_handler(
                &State::EndAccept,
                shared_from_this()));
    }

    void EndAccept(boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Error when accepting HTTP client: " << ec.message();
        }
        else
        {
            BOOST_LOG_TRIVIAL(debug) << "Incoming HTTP connection from " << socket.remote_endpoint();

            auto session = std::make_shared<HttpSession>(
                std::move(socket),
                m_middlewares);

            m_sessions.push_back(session);

            session->Run();
        }

        BeginAccept();
    }

    boost::asio::io_context& m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::vector<std::weak_ptr<HttpSession>> m_sessions;
    std::vector<porla::HttpMiddleware> m_middlewares;
};

HttpServer::HttpServer(boost::asio::io_context& io, porla::HttpServerOptions const& options)
    : m_io(io)
{
    m_state = std::make_shared<State>(io, options.host, options.port);
    m_state->Start();
}

HttpServer::~HttpServer()
{
    m_state->Stop();
}

boost::asio::ip::tcp::endpoint HttpServer::Endpoint()
{
    return m_state->Endpoint();
}

void HttpServer::Use(const porla::HttpMiddleware& middleware)
{
    m_state->Use(middleware);
}
