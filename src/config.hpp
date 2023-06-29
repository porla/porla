#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>

#include <boost/program_options.hpp>
#include <libtorrent/extensions.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/storage_defs.hpp>
#include <sqlite3.h>
#include <toml++/toml.h>

typedef std::function<std::shared_ptr<libtorrent::torrent_plugin>(libtorrent:: torrent_handle const&, libtorrent::client_data_t)> lt_plugin;

namespace fs = std::filesystem;

namespace porla
{
    class Config
    {
    public:
        struct Preset
        {
            std::optional<std::string>                category;
            std::optional<int>                        download_limit;
            std::optional<int>                        max_connections;
            std::optional<int>                        max_uploads;
            std::optional<std::string>                save_path;
            std::optional<libtorrent::storage_mode_t> storage_mode;
            std::unordered_set<std::string>           tags;
            std::optional<int>                        upload_limit;
        };

        std::optional<std::string>            config_file;
        toml::table                           config_tbl;
        sqlite3*                              db;
        std::optional<std::string>            db_file;
        std::optional<bool>                   http_auth_enabled;
        std::optional<std::string>            http_base_path;
        std::optional<std::string>            http_host;
        std::optional<bool>                   http_metrics_enabled;
        std::optional<uint16_t>               http_port;
        std::optional<bool>                   http_webui_enabled;

        std::optional<bool>                   plugins_allow_git;
        std::optional<fs::path>               plugins_install_dir;
        std::map<std::string, Preset>         presets;
        std::string                           secret_key;
        std::optional<std::vector<lt_plugin>> session_extensions;
        libtorrent::settings_pack             session_settings;
        std::optional<int>                    sodium_memlimit;
        std::optional<fs::path>               state_dir;
        std::optional<int>                    timer_dht_stats;
        std::optional<int>                    timer_session_stats;
        std::optional<int>                    timer_torrent_updates;
        std::optional<fs::path>               workflow_dir;

        static std::unique_ptr<Config> Load(const boost::program_options::variables_map& cmd);

        ~Config();

        Config(const Config&) = delete;
        Config(const Config&&) = delete;
        Config& operator=(const Config&) = delete;
        Config& operator=(Config&&) = delete;

    private:
        explicit Config() = default;
    };
}
