#pragma once

#include <map>
#include <string>
#include <vector>

#include "httpcontext.hpp"

namespace porla
{
    class EmbeddedWebUIHandler
    {
    public:
        explicit EmbeddedWebUIHandler();

        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        std::map<std::string, std::vector<char>> m_files;
    };
}
