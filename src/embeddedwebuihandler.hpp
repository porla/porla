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
        explicit EmbeddedWebUIHandler(std::string base_path);

        void operator()(const std::shared_ptr<HttpContext>&);

    private:
        std::string m_base_path;
        std::map<std::string, std::vector<char>> m_files;
    };
}
