#pragma once

#include <memory>
#include <optional>
#include <string>

#include <libtorrent/extensions.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/storage_defs.hpp>
#include <sqlite3.h>

typedef std::function<std::shared_ptr<libtorrent::torrent_plugin>(libtorrent:: torrent_handle const&, libtorrent::client_data_t)> lt_plugin;

namespace porla
{
    class Config
    {
    public:
        struct Preset
        {
            std::optional<int>                        download_limit;
            std::optional<int>                        max_connections;
            std::optional<int>                        max_uploads;
            std::optional<std::string>                save_path;
            std::optional<libtorrent::storage_mode_t> storage_mode;
            std::optional<int>                        upload_limit;
        };

        std::optional<std::string>            config_file;
        sqlite3*                              db;
        std::optional<std::string>            db_file;
        std::optional<std::string>            http_auth_token;
        std::optional<std::string>            http_host;
        std::optional<bool>                   http_metrics_enabled;
        std::optional<uint16_t>               http_port;
        std::optional<bool>                   http_webui_enabled;
        std::optional<std::string>            log_level;
        std::map<std::string, Preset>         presets;
        std::optional<std::vector<lt_plugin>> session_extensions;
        libtorrent::settings_pack             session_settings;
        std::optional<int>                    timer_dht_stats;
        std::optional<int>                    timer_session_stats;
        std::optional<int>                    timer_torrent_updates;

        static std::unique_ptr<Config> Load(int argc, char* argv[]);

        ~Config();

        Config(const Config&) = delete;
        Config(const Config&&) = delete;
        Config& operator=(const Config&) = delete;
        Config& operator=(Config&&) = delete;

    private:
        explicit Config() = default;
    };
}
