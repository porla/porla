#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "handler.hpp"

namespace porla::Http
{
    class WebUIHandler
    {
    public:
        explicit WebUIHandler(const std::filesystem::path& webui_file, std::string base_path);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        std::string m_base_path;
        std::map<std::string, std::vector<char>> m_files;
        std::filesystem::path m_webui_file;
    };
}
