#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <sqlite3.h>

namespace porla
{
    struct SessionsOptions
    {
        sqlite3*                 db;
        boost::asio::io_context& io;
    };

    struct SessionsLoadOptions
    {
        std::string           name;
        std::filesystem::path session_params_file;
        lt::settings_pack     settings;
    };

    class Sessions
    {
    public:
        struct SessionState
        {
            std::string                                   name;
            std::shared_ptr<lt::session>                  session;
            std::map<lt::info_hash_t, lt::torrent_handle> torrents;
        };

        explicit Sessions(const SessionsOptions& options);

        std::shared_ptr<SessionState> Default();
        std::shared_ptr<SessionState> Get(const std::string& name);

        void Load(const SessionsLoadOptions& options);

    private:
        void ReadAlerts(const std::shared_ptr<SessionState>& state);

        SessionsOptions m_options;
        std::map<std::string, std::shared_ptr<SessionState>> m_sessions;
    };
}
