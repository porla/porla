#pragma once

#include <memory>
#include <thread>
#include <uWebSockets/HttpParser.h>
#include <vector>

#include <boost/asio.hpp>
#include <sqlite3.h>

#include "handler.hpp"

namespace porla::Http
{
    struct AuthLoginHandlerOptions
    {
        sqlite3*                 db;
        boost::asio::io_context& io;
        std::string              secret_key;
    };

    class AuthLoginHandler
    {
    public:
        explicit AuthLoginHandler(const AuthLoginHandlerOptions& opts);
        ~AuthLoginHandler();

        void operator()(uWS::HttpResponse<true> *res, uWS::HttpRequest *req);
        void operator()(uWS::HttpResponse<false> *res, uWS::HttpRequest *req);

    private:
        struct State;
        std::shared_ptr<State> m_state;
        template <bool SSL> void callHandler(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req);
    };
} // namespace porla::Http
