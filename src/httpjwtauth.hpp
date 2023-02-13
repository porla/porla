#pragma once

#include <optional>

#include "httpcontext.hpp"
#include "httpmiddleware.hpp"

namespace porla
{
    class HttpJwtAuth
    {
    public:
        explicit HttpJwtAuth(std::string secret_key, std::optional<std::string> alt_header, HttpMiddleware middleware);

        void operator()(const std::shared_ptr<porla::HttpContext>& ctx);

    private:
        std::string m_secret_key;
        std::optional<std::string> m_alt_header;
        HttpMiddleware m_http_middleware;
    };
}
