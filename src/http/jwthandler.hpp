#pragma once

#include <functional>
#include <string>

#include "handler.hpp"

namespace porla::Http
{
    template <bool SSL>
    class JwtHandler
    {
    public:
        explicit JwtHandler(const std::string& secret_key, Handler<SSL> next);

        void operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req);

    private:
        std::string m_secret_key;
        Handler<SSL> m_next;
    };
}
