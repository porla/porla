#include "config.hpp"

#include <filesystem>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/fingerprint.hpp>
#include <libtorrent/session.hpp>
#include <toml++/toml.h>

#include "data/migrate.hpp"
#include "utils/secretkey.hpp"

namespace fs = std::filesystem;
namespace lt = libtorrent;
namespace po = boost::program_options;

using porla::Config;

static void ApplySettings(const toml::table& tbl, lt::settings_pack& settings);

std::unique_ptr<Config> Config::Load(const boost::program_options::variables_map& cmd)
{
    const static std::vector<fs::path> config_file_search_paths =
    {
        fs::current_path() / "porla.toml",

        // Check $XDG_CONFIG_HOME/porla/porla.toml
        std::getenv("XDG_CONFIG_HOME")
            ? fs::path(std::getenv("XDG_CONFIG_HOME")) / "porla" / "porla.toml"
            : fs::path(),

        // Check $HOME/.config/porla/porla.toml
        std::getenv("HOME")
            ? fs::path(std::getenv("HOME")) / ".config" / "porla" / "porla.toml"
            : fs::path(),

        // Check $HOME/.config/porla.toml
        std::getenv("HOME")
            ? fs::path(std::getenv("HOME")) / ".config" / "porla.toml"
            : fs::path(),

        "/etc/porla/porla.toml",
        "/etc/porla.toml"
    };

    auto cfg = std::unique_ptr<Config>(new Config());
    cfg->http_auth_enabled         = true;
    cfg->session_settings          = lt::default_settings();

    // Check default locations for a config file.
    for (auto const& path : config_file_search_paths)
    {
        // Skip empty paths.
        if (path == fs::path()) continue;

        std::error_code ec;
        bool regular_file = fs::is_regular_file(path, ec);

        if (ec || !regular_file)
        {
            continue;
        }

        cfg->config_file = path;

        break;
    }

    if (auto val = std::getenv("PORLA_CONFIG_FILE"))           cfg->config_file     = val;
    if (auto val = std::getenv("PORLA_DB"))                    cfg->db_file         = val;
    if (auto val = std::getenv("PORLA_HTTP_AUTH_DISABLED_YES_REALLY"))
    {
        if (strcmp("true", val) == 0) cfg->http_auth_enabled = false;
    }
    if (auto val = std::getenv("PORLA_HTTP_BASE_PATH"))        cfg->http_base_path  = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg->http_host       = val;
    if (auto val = std::getenv("PORLA_HTTP_METRICS_ENABLED"))
    {
        if (strcmp("true", val) == 0)  cfg->http_metrics_enabled = true;
        if (strcmp("false", val) == 0) cfg->http_metrics_enabled = false;
    }
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg->http_port       = std::stoi(val);
    if (auto val = std::getenv("PORLA_HTTP_WEBUI_ENABLED"))
    {
        if (strcmp("true", val) == 0)  cfg->http_webui_enabled = true;
        if (strcmp("false", val) == 0) cfg->http_webui_enabled = false;
    }
    if (auto val = std::getenv("PORLA_SECRET_KEY"))            cfg->secret_key      = val;
    if (auto val = std::getenv("PORLA_SESSION_SETTINGS_BASE"))
    {
        if (strcmp("default", val) == 0)               cfg->session_settings = lt::default_settings();
        if (strcmp("high_performance_seed", val) == 0) cfg->session_settings = lt::high_performance_seed();
        if (strcmp("min_memory_usage", val) == 0)      cfg->session_settings = lt::min_memory_usage();
    }
    if (auto val = std::getenv("PORLA_STATE_DIR"))             cfg->state_dir             = val;
    if (auto val = std::getenv("PORLA_TIMER_DHT_STATS"))       cfg->timer_dht_stats       = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_SESSION_STATS"))   cfg->timer_session_stats   = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_TORRENT_UPDATES")) cfg->timer_torrent_updates = std::stoi(val);
    if (auto val = std::getenv("PORLA_WORKFLOW_DIR"))          cfg->workflow_dir          = val;

    if (cmd.count("config-file"))
    {
        cfg->config_file = cmd["config-file"].as<std::string>();

        if (!fs::is_regular_file(cfg->config_file.value()))
        {
            BOOST_LOG_TRIVIAL(warning)
                << "User-specified config file does not exist: "
                << fs::absolute(cfg->config_file.value());
        }
    }

    // Apply configuration from the config file before we apply the command line args.
    if (cfg->config_file && fs::is_regular_file(cfg->config_file.value()))
    {
        BOOST_LOG_TRIVIAL(debug) << "Reading config file at " << cfg->config_file.value();

        std::ifstream config_file_data(cfg->config_file.value(), std::ios::binary);

        try
        {
            const toml::table config_file_tbl = toml::parse(config_file_data);

            if (auto val = config_file_tbl["db"].value<std::string>())
                cfg->db_file = *val;

            if (const auto listen_interfaces_array = config_file_tbl["listen_interfaces"].as_array())
            {
                std::stringstream listen;
                std::stringstream outbound;

                for (const auto& listen_interface_item : *listen_interfaces_array)
                {
                    if (!listen_interface_item.is_array())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Listen interface item is not an array";
                        continue;
                    }

                    const auto& item_array = listen_interface_item.as_array();

                    if (item_array->size() != 2)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Listen interface item of wrong size";
                        continue;
                    }

                    const auto& item_dev = item_array->at(0);
                    const auto& item_port = item_array->at(1);

                    if (item_dev.type() != toml::node_type::string)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Listen interface device is not a string";
                        continue;
                    }

                    if (item_port.type() != toml::node_type::integer)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Listen interface port is not an integer";
                        continue;
                    }

                    listen << "," << *item_dev.value<std::string>() << ":" << *item_port.value<int>();
                    outbound << "," << *item_dev.value<std::string>();
                }

                const std::string listen_val = listen.str();
                const std::string outbound_val = outbound.str();

                if (!listen_val.empty())
                {
                    cfg->session_settings.set_str(lt::settings_pack::listen_interfaces, listen_val.substr(1));
                }

                if (!outbound_val.empty())
                {
                    cfg->session_settings.set_str(lt::settings_pack::outgoing_interfaces, outbound_val.substr(1));
                }
            }

            if (auto val = config_file_tbl["http"]["base_path"].value<std::string>())
                cfg->http_base_path = *val;

            if (auto val = config_file_tbl["http"]["host"].value<std::string>())
                cfg->http_host = *val;

            if (auto val = config_file_tbl["http"]["metrics_enabled"].value<bool>())
                cfg->http_metrics_enabled = *val;

            if (auto val = config_file_tbl["http"]["port"].value<uint16_t>())
                cfg->http_port = *val;

            if (auto val = config_file_tbl["http"]["webui_enabled"].value<bool>())
                cfg->http_webui_enabled = *val;

            // Load presets
            if (auto const* presets_tbl = config_file_tbl["presets"].as_table())
            {
                for (auto const [key,value] : *presets_tbl)
                {
                    if (!value.is_table())
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Preset '" << key << "' is not a TOML table";
                        continue;
                    }

                    const toml::table value_tbl = *value.as_table();

                    Preset p = {};

                    if (auto val = value_tbl["category"].value<std::string>())
                        p.category = *val;

                    if (auto val = value_tbl["download_limit"].value<int>())
                        p.download_limit = *val;

                    if (auto val = value_tbl["max_connections"].value<int>())
                        p.max_connections = *val;

                    if (auto val = value_tbl["max_uploads"].value<int>())
                        p.max_uploads = *val;

                    if (auto val = value_tbl["save_path"].value<std::string>())
                        p.save_path = *val;

                    if (auto val = value_tbl["storage_mode"].value<std::string>())
                    {
                        if (strcmp(val->c_str(), "allocate") == 0) p.storage_mode = lt::storage_mode_allocate;
                        if (strcmp(val->c_str(), "sparse") == 0)   p.storage_mode = lt::storage_mode_sparse;
                    }

                    if (auto const tags_val = value_tbl["tags"].as_array())
                    {
                        for (auto const& tag_item : *tags_val)
                        {
                            if (auto const tag_value = tag_item.value<std::string>())
                            {
                                p.tags.insert(*tag_value);
                            }
                        }
                    }

                    if (auto val = value_tbl["upload_limit"].value<int>())
                        p.upload_limit = *val;

                    cfg->presets.insert({ key.data(), std::move(p) });
                }
            }

            if (const auto proxy_tbl = config_file_tbl["proxy"].as_table())
            {
                BOOST_LOG_TRIVIAL(info) << "Configuring session proxy";

                if (const auto val = (*proxy_tbl)["host"].value<std::string>())
                    cfg->session_settings.set_str(lt::settings_pack::proxy_hostname, *val);

                if (const auto val = (*proxy_tbl)["port"].value<int>())
                    cfg->session_settings.set_int(lt::settings_pack::proxy_port, *val);

                if (const auto val = (*proxy_tbl)["type"].value<std::string>())
                {
                    if (strcmp(val->c_str(), "socks4") == 0 || strcmp(val->c_str(), "SOCKS4") == 0)
                        cfg->session_settings.set_int(lt::settings_pack::proxy_type, lt::settings_pack::socks4);

                    if (strcmp(val->c_str(), "socks5") == 0 || strcmp(val->c_str(), "SOCKS5") == 0)
                        cfg->session_settings.set_int(lt::settings_pack::proxy_type, lt::settings_pack::socks5);
                }

                if (const auto val = (*proxy_tbl)["hostnames"].value<bool>())
                    cfg->session_settings.set_bool(lt::settings_pack::proxy_hostnames, *val);

                if (const auto val = (*proxy_tbl)["peer_connections"].value<bool>())
                    cfg->session_settings.set_bool(lt::settings_pack::proxy_peer_connections, *val);

                if (const auto val = (*proxy_tbl)["tracker_connections"].value<bool>())
                    cfg->session_settings.set_bool(lt::settings_pack::proxy_tracker_connections, *val);
            }

            if (auto val = config_file_tbl["secret_key"].value<std::string>())
                cfg->secret_key = *val;

            if (auto val = config_file_tbl["session_settings"]["extensions"].as_array())
            {
                std::vector<lt_plugin> extensions;

                for (auto const& item : *val)
                {
                    if (auto const item_value = item.value<std::string>())
                    {
                        if (*item_value == "smart_ban")
                            extensions.emplace_back(&lt::create_smart_ban_plugin);

                        if (*item_value == "ut_metadata")
                            extensions.emplace_back(&lt::create_ut_metadata_plugin);

                        if (*item_value == "ut_pex")
                            extensions.emplace_back(&lt::create_ut_pex_plugin);
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(warning)
                            << "Item in session_extension array is not a string (" << item.type() << ")";
                    }
                }

                cfg->session_extensions = extensions;
            }

            if (auto val = config_file_tbl["session_settings"]["base"].value<std::string>())
            {
                if (*val == "default")               cfg->session_settings = lt::default_settings();
                if (*val == "high_performance_seed") cfg->session_settings = lt::high_performance_seed();
                if (*val == "min_memory_usage")      cfg->session_settings = lt::min_memory_usage();
            }

            if (auto session_settings_tbl = config_file_tbl["session_settings"].as_table())
                ApplySettings(*session_settings_tbl, cfg->session_settings);

            if (auto val = config_file_tbl["state_dir"].value<std::string>())
                cfg->state_dir = *val;

            if (auto val = config_file_tbl["timer"]["dht_stats"].value<int>())
                cfg->timer_dht_stats = *val;

            if (auto val = config_file_tbl["timer"]["session_stats"].value<int>())
                cfg->timer_session_stats = *val;

            if (auto val = config_file_tbl["timer"]["torrent_updates"].value<int>())
                cfg->timer_torrent_updates = *val;

            if (auto val = config_file_tbl["workflow_dir"].value<std::string>())
                cfg->workflow_dir = *val;
        }
        catch (const toml::parse_error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse config file '" << cfg->config_file.value() << "': " << err;
            throw;
        }
    }

    if (cmd.count("db"))                    cfg->db_file               = cmd["db"].as<std::string>();
    if (cmd.count("http-base-path"))        cfg->http_base_path        = cmd["http-base-path"].as<std::string>();
    if (cmd.count("http-host"))             cfg->http_host             = cmd["http-host"].as<std::string>();
    if (cmd.count("http-metrics-enabled"))
    {
        cfg->http_metrics_enabled = cmd["http-metrics-enabled"].as<bool>();
    }
    if (cmd.count("http-port"))             cfg->http_port             = cmd["http-port"].as<uint16_t>();
    if (cmd.count("http-webui-enabled"))
    {
        cfg->http_webui_enabled = cmd["http-webui-enabled"].as<bool>();
    }
    if (cmd.count("secret-key"))            cfg->secret_key            = cmd["secret-key"].as<std::string>();
    if (cmd.count("session-settings-base"))
    {
        auto val = cmd["session-settings-base"].as<std::string>();

        if (val == "default")               cfg->session_settings = lt::default_settings();
        if (val == "high_performance_seed") cfg->session_settings = lt::high_performance_seed();
        if (val == "min_memory_usage")      cfg->session_settings = lt::min_memory_usage();
    }
    if (cmd.count("state-dir"))             cfg->state_dir             = cmd["state-dir"].as<std::string>();
    if (cmd.count("timer-dht-stats"))       cfg->timer_dht_stats       = cmd["timer-dht-stats"].as<int>();
    if (cmd.count("timer-session-stats"))   cfg->timer_session_stats   = cmd["timer-session-stats"].as<pid_t>();
    if (cmd.count("timer-torrent-updates")) cfg->timer_torrent_updates = cmd["timer-torrent-updates"].as<pid_t>();
    if (cmd.count("workflow-dir"))          cfg->workflow_dir          = cmd["workflow-dir"].as<std::string>();

    // If no db_file is set, default to a file in state_dir.
    if (!cfg->db_file.has_value())
    {
        cfg->db_file = cfg->state_dir.value_or(fs::current_path()) / "porla.sqlite";
    }

    if (sqlite3_open(cfg->db_file.value_or("porla.sqlite").c_str(), &cfg->db) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to open SQLite connection: " << sqlite3_errmsg(cfg->db);
        throw std::runtime_error("Failed to open SQLite connection");
    }

    if (sqlite3_exec(cfg->db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to enable WAL journal mode: " << sqlite3_errmsg(cfg->db);
        throw std::runtime_error("Failed to enable WAL journal mode");
    }

    if (!porla::Data::Migrate(cfg->db))
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to run migrations";
        throw std::runtime_error("Failed to apply migrations");
    }

    // Apply static libtorrent settings here. These are always set after all other settings from
    // the config are applied, and cannot be overwritten by it.
    lt::alert_category_t alerts =
        lt::alert::status_notification
        | lt::alert::storage_notification
        | lt::alert::tracker_notification;

    cfg->session_settings.set_int(lt::settings_pack::alert_mask, alerts);
    cfg->session_settings.set_str(lt::settings_pack::peer_fingerprint, lt::generate_fingerprint("PO", 0, 1));
    cfg->session_settings.set_str(lt::settings_pack::user_agent, "porla/1.0");

    // If we get here without having a secret key, we must generate one. Also log a warning because
    // if the secret key changes, JWT's will not work if restarting.

    if (cfg->secret_key.empty())
    {
        BOOST_LOG_TRIVIAL(warning) << "No secret key set. Porla will generate one";
        BOOST_LOG_TRIVIAL(warning) << "Use './porla key:generate' to generate a secret key";

        cfg->secret_key = porla::Utils::SecretKey::New();
    }

    return std::move(cfg);
}

Config::~Config()
{
    if (db != nullptr)
    {
        BOOST_LOG_TRIVIAL(debug) << "Vacuuming database";

        if (sqlite3_exec(db, "VACUUM;", nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to vacuum database: " << sqlite3_errmsg(db);
        }

        if (sqlite3_close(db) != SQLITE_OK)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to close SQLite connection: " << sqlite3_errmsg(db);
        }
    }
}

static void ApplySettings(const toml::table& tbl, lt::settings_pack& settings)
{
    static const std::unordered_set<std::string> BlockedKeys =
    {
        "peer_fingerprint",
        "user_agent"
    };

    for (auto const& [key,value] : tbl)
    {
        const int type = lt::setting_by_name(key.data());

        if (type == -1)
        {
            continue;
        }

        if (BlockedKeys.contains(key.data()))
        {
            continue;
        }

        if ((type & lt::settings_pack::type_mask) == lt::settings_pack::bool_type_base)
        {
            if (!value.is_boolean())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not a boolean";
                continue;
            }

            settings.set_bool(type, *value.value<bool>());
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::int_type_base)
        {
            if (!value.is_integer())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not an integer";
                continue;
            }

            settings.set_int(type, *value.value<int>());
        }
        else if((type & lt::settings_pack::type_mask) == lt::settings_pack::string_type_base)
        {
            if (!value.is_string())
            {
                BOOST_LOG_TRIVIAL(warning) << "Value for setting " << key << " is not a string";
                continue;
            }

            settings.set_str(type, *value.value<std::string>());
        }
    }
}
