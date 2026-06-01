#pragma once

#include <functional>
#include <string>

#include "handler.hpp"

namespace porla::Http
{
    class JwtHandler
    {
    public:
        explicit JwtHandler(const std::string& secret_key, SecureHandler next);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        std::string m_secret_key;
        SecureHandler m_next;
    };
}
