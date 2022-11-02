#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include "httpmiddleware.hpp"

namespace porla
{
    struct HttpServerOptions
    {
        std::string host;
        uint16_t port;
    };

    class HttpServer
    {
    public:
        HttpServer(boost::asio::io_context& io, HttpServerOptions const& options);
        ~HttpServer();

        boost::asio::ip::tcp::endpoint Endpoint();
        void Use(const HttpMiddleware& middleware);

    private:
        class State;

        boost::asio::io_context& m_io;
        std::shared_ptr<State> m_state;
    };
}
