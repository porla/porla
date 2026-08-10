#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <boost/signals2.hpp>
#include <sqlite3.h>

#include "handler.hpp"

namespace porla::Http
{
    class WebUIHandler
    {
    public:
        explicit WebUIHandler(sqlite3* db, std::string base_path, boost::signals2::signal<void(const std::unordered_set<std::string>&)>& reload_signal);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        void LoadUI();

        sqlite3* m_db;
        std::string m_base_path;
        std::map<std::string, std::vector<char>> m_files;
        std::filesystem::path m_webui_file;
        boost::signals2::connection m_reload_connection;
    };
}
