#pragma once

#include <map>
#include <string>
#include <vector>

#include "handler.hpp"

namespace porla::Http
{
    template <bool SSL> class WebUIHandler
    {
    public:
        explicit WebUIHandler(std::string base_path);

        void operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req);

    private:
        std::string m_base_path;
        std::map<std::string, std::vector<char>> m_files;
    };
}
