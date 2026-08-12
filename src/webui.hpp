#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <sqlite3.h>
#include <uWebSockets/App.h>

namespace fs = std::filesystem;

namespace porla
{
    class CurlMulti;

    class WebUI : public std::enable_shared_from_this<WebUI>
    {
    public:
        static std::shared_ptr<WebUI> Create(
            boost::asio::io_context& io,
            fs::path state_dir,
            sqlite3* db,
            std::weak_ptr<CurlMulti> cm);

        bool Has();
        void Install(const std::string& version, std::function<void()> callback = {});

        std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> HttpHandler();

    private:
        explicit WebUI(boost::asio::io_context& io, fs::path state_dir, sqlite3* db, std::weak_ptr<CurlMulti> cm);

        void HttpGet(const std::string& url, std::function<void(int, std::string)> callback);
        void LoadCurrent();

        boost::asio::io_context& m_io;
        fs::path m_state_dir;
        sqlite3* m_db;
        std::weak_ptr<CurlMulti> m_cm;
        std::map<std::string, std::vector<char>> m_files;
        std::string m_base_path;
    };
}
