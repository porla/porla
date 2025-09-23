#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <boost/signals2.hpp>

#include "handler.hpp"

namespace porla::Http
{
    class WebUIHandler
    {
    public:
        explicit WebUIHandler(const std::filesystem::path& webui_file, std::string base_path, boost::signals2::signal<void(const char*, size_t)>& reload_signal);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        void LoadUI();
        void LoadUIFromBuffer(const char* buffer, size_t size);

        std::string m_base_path;
        std::map<std::string, std::vector<char>> m_files;
        std::filesystem::path m_webui_file;
        boost::signals2::connection m_reload_connection;
    };
}
