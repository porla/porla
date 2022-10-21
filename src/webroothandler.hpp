#pragma once

#include <string>

#include "httpcontext.hpp"

namespace porla
{
    class WebRootHandler
    {
    public:
        explicit WebRootHandler(std::string root);

        void operator()(const std::shared_ptr<porla::HttpContext>& ctx);

    private:
        std::string m_root;
    };
}
