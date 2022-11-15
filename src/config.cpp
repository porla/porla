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
#include "data/models/sessionsettings.hpp"

namespace fs = std::filesystem;
namespace lt = libtorrent;
namespace po = boost::program_options;

using porla::Config;

std::unique_ptr<Config> Config::Load(int argc, char **argv)
{
    const static std::vector<fs::path> config_file_search_paths =
    {
        fs::current_path() / "porla.toml",
        "/etc/porla/porla.toml",
        "/etc/porla.toml"
    };

    auto cfg = std::unique_ptr<Config>(new Config());
    cfg->session_settings = lt::default_settings();

    // Check default locations for a config file.
    for (auto const& path : config_file_search_paths)
    {
        std::error_code ec;
        bool regular_file = fs::is_regular_file(path, ec);

        if (ec || !regular_file)
        {
            continue;
        }

        cfg->config_file = path;
    }

    if (auto val = std::getenv("PORLA_CONFIG_FILE"))           cfg->config_file     = val;
    if (auto val = std::getenv("PORLA_DB"))                    cfg->db_file         = val;
    if (auto val = std::getenv("PORLA_HTTP_AUTH_TOKEN"))       cfg->http_auth_token = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg->http_host       = val;
    if (auto val = std::getenv("PORLA_HTTP_METRICS_ENABLED"))
    {
        if (strcmp("true", val) == 0)  cfg->http_metrics_enabled = true;
        if (strcmp("false", val) == 0) cfg->http_metrics_enabled = false;
    }
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg->http_port       = std::stoi(val);
    if (auto val = std::getenv("PORLA_LOG_LEVEL"))             cfg->log_level       = val;
    if (auto val = std::getenv("PORLA_SESSION_SETTINGS_BASE"))
    {
        if (strcmp("default", val) == 0)               cfg->session_settings = lt::default_settings();
        if (strcmp("high_performance_seed", val) == 0) cfg->session_settings = lt::high_performance_seed();
        if (strcmp("min_memory_usage", val) == 0)      cfg->session_settings = lt::min_memory_usage();
    }
    if (auto val = std::getenv("PORLA_TIMER_DHT_STATS"))       cfg->timer_dht_stats       = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_SESSION_STATS"))   cfg->timer_session_stats   = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_TORRENT_UPDATES")) cfg->timer_torrent_updates = std::stoi(val);

    po::options_description desc("Allowed options");
    desc.add_options()
        ("config-file",           po::value<std::string>(), "Path to a porla.toml config file.")
        ("db",                    po::value<std::string>(), "Path to where the database will be stored.")
        ("help",                                            "Show usage")
        ("http-auth-token",       po::value<std::string>(), "The auth token to use for the HTTP server.")
        ("http-host",             po::value<std::string>(), "The host to listen on for HTTP traffic.")
        ("http-metrics-enabled",  po::value<bool>(),        "Set to true if the metrics endpoint should be enabled")
        ("http-port",             po::value<uint16_t>(),    "The port to listen on for HTTP traffic.")
        ("log-level",             po::value<std::string>(), "The minimum log level to print.")
        ("session-settings-base", po::value<std::string>(), "The libtorrent base settings to use")
        ("supervised-interval",   po::value<int>(),         "The interval to use when checking the supervisor pid.")
        ("supervised-pid",        po::value<pid_t>(),       "A pid to a parent process. If this pid dies, we shut down.")
        ("timer-dht-stats",       po::value<int>(),         "The interval to use for the DHT stats updates.")
        ("timer-session-stats",   po::value<int>(),         "The interval to use for the session stats updates.")
        ("timer-torrent-updates", po::value<int>(),         "The interval to use for the torrent updates.")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc;
        exit(0); // TODO: Not ideal.
    }

    if (vm.count("config-file"))
    {
        cfg->config_file = vm["config-file"].as<std::string>();

        if (!fs::is_regular_file(cfg->config_file.value()))
        {
            BOOST_LOG_TRIVIAL(warning) << "User-specified config file does not exist: " << cfg->config_file.value();
        }
    }

    // Apply configuration from the config file before we apply the command line args.
    if (cfg->config_file && fs::is_regular_file(cfg->config_file.value()))
    {
        std::ifstream config_file_data(cfg->config_file.value(), std::ios::binary);

        try
        {
            const toml::table config_file_tbl = toml::parse(config_file_data);

            if (auto val = config_file_tbl["db"].value<std::string>())
                cfg->db_file = *val;

            if (auto val = config_file_tbl["http"]["auth_token"].value<std::string>())
                cfg->http_auth_token = *val;

            if (auto val = config_file_tbl["http"]["host"].value<std::string>())
                cfg->http_host = *val;

            if (auto val = config_file_tbl["http"]["metrics_enabled"].value<bool>())
                cfg->http_metrics_enabled = *val;

            if (auto val = config_file_tbl["http"]["port"].value<uint16_t>())
                cfg->http_port = *val;

            if (auto val = config_file_tbl["log_level"].value<std::string>())
                cfg->log_level = *val;

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

                    if (auto val = value_tbl["upload_limit"].value<int>())
                        p.upload_limit = *val;

                    cfg->presets.insert({ key.data(), std::move(p) });
                }
            }

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

            if (auto val = config_file_tbl["timer"]["dht_stats"].value<int>())
                cfg->timer_dht_stats = *val;

            if (auto val = config_file_tbl["timer"]["session_stats"].value<int>())
                cfg->timer_session_stats = *val;

            if (auto val = config_file_tbl["timer"]["torrent_updates"].value<int>())
                cfg->timer_torrent_updates = *val;
        }
        catch (const toml::parse_error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse config file '" << cfg->config_file.value() << "': " << err;
        }
    }

    if (vm.count("db"))                    cfg->db_file               = vm["db"].as<std::string>();
    if (vm.count("http-auth-token"))       cfg->http_auth_token       = vm["http-auth-token"].as<std::string>();
    if (vm.count("http-host"))             cfg->http_host             = vm["http-host"].as<std::string>();
    if (vm.count("http-metrics-enabled"))
    {
        cfg->http_metrics_enabled = vm["http-metrics-enabled"].as<bool>();
    }
    if (vm.count("http-port"))             cfg->http_port             = vm["http-port"].as<uint16_t>();
    if (vm.count("log-level"))             cfg->log_level             = vm["log-level"].as<std::string>();
    if (vm.count("session-settings-base"))
    {
        auto val = vm["session-settings-base"].as<std::string>();

        if (val == "default")               cfg->session_settings = lt::default_settings();
        if (val == "high_performance_seed") cfg->session_settings = lt::high_performance_seed();
        if (val == "min_memory_usage")      cfg->session_settings = lt::min_memory_usage();
    }
    if (vm.count("timer-dht-stats"))       cfg->timer_dht_stats       = vm["timer-dht-stats"].as<int>();
    if (vm.count("timer-session-stats"))   cfg->timer_session_stats   = vm["timer-session-stats"].as<pid_t>();
    if (vm.count("timer-torrent-updates")) cfg->timer_torrent_updates = vm["timer-torrent-updates"].as<pid_t>();

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

    porla::Data::Models::SessionSettings::Apply(cfg->db, cfg->session_settings);

    // Apply static libtorrent settings here. These are always set after all other settings from
    // the config are applied, and cannot be overwritten by it.
    cfg->session_settings.set_str(lt::settings_pack::peer_fingerprint, lt::generate_fingerprint("PO", 0, 1));
    cfg->session_settings.set_str(lt::settings_pack::user_agent, "porla/1.0");

    return std::move(cfg);
}

Config::~Config()
{
    BOOST_LOG_TRIVIAL(info) << "Vacuuming database";

    if (sqlite3_exec(db, "VACUUM;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to vacuum database: " << sqlite3_errmsg(db);
    }

    if (sqlite3_close(db) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to close SQLite connection: " << sqlite3_errmsg(db);
    }
}
