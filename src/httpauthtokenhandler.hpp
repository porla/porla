#pragma once

#include <memory>
#include <string>

#include "httpcontext.hpp"

namespace porla
{
    class HttpAuthTokenHandler
    {
    public:
        explicit HttpAuthTokenHandler(std::string token);

        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        std::string m_token;
    };
}
