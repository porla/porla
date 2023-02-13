#pragma once

#include <optional>

#include "httpcontext.hpp"
#include "httpmiddleware.hpp"

namespace porla
{
    class HttpJwtAuth
    {
    public:
        explicit HttpJwtAuth(std::string secret_key, HttpMiddleware middleware);

        void operator()(const std::shared_ptr<porla::HttpContext>& ctx);

    private:
        std::string m_secret_key;
        HttpMiddleware m_http_middleware;
    };
}
