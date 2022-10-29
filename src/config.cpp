#include "config.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <libtorrent/session.hpp>
#include <toml++/toml.h>

namespace fs = std::filesystem;
namespace lt = libtorrent;
namespace po = boost::program_options;

using porla::Config;

Config Config::Load(int argc, char **argv)
{
    const static std::vector<fs::path> config_file_search_paths =
    {
        fs::current_path() / "porla.toml",
        "/etc/porla/porla.toml",
        "/etc/porla.toml"
    };

    Config cfg = {};
    cfg.session_settings = lt::default_settings();

    // Check default locations for a config file.
    for (auto const& path : config_file_search_paths)
    {
        std::error_code ec;
        bool regular_file = fs::is_regular_file(path, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to check config file path: " << path;
            continue;
        }

        if (regular_file)
        {
            cfg.config_file = path;
            break;
        }
    }

    if (auto val = std::getenv("PORLA_CONFIG_FILE"))           cfg.config_file = val;
    if (auto val = std::getenv("PORLA_DB"))                    cfg.db          = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg.http_host   = val;
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg.http_port   = std::stoi(val);
    if (auto val = std::getenv("PORLA_LOG_LEVEL"))             cfg.log_level   = val;
    if (auto val = std::getenv("PORLA_SESSION_SETTINGS_BASE"))
    {
        if (strcmp("default", val) == 0)               cfg.session_settings = lt::default_settings();
        if (strcmp("high_performance_seed", val) == 0) cfg.session_settings = lt::high_performance_seed();
        if (strcmp("min_memory_usage", val) == 0)      cfg.session_settings = lt::min_memory_usage();
    }
    if (auto val = std::getenv("PORLA_SUPERVISED_INTERVAL"))   cfg.supervised_interval   = std::stoi(val);
    if (auto val = std::getenv("PORLA_SUPERVISED_PID"))        cfg.supervised_pid        = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_DHT_STATS"))       cfg.timer_dht_stats       = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_SESSION_STATS"))   cfg.timer_session_stats   = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_TORRENT_UPDATES")) cfg.timer_torrent_updates = std::stoi(val);

    po::options_description desc("Allowed options");
    desc.add_options()
        ("config-file",           po::value<std::string>(), "Path to a porla.toml config file.")
        ("db",                    po::value<std::string>(), "Path to where the database will be stored.")
        ("http-host",             po::value<std::string>(), "The host to listen on for HTTP traffic.")
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

    if (vm.count("config-file"))
    {
        cfg.config_file = vm["config-file"].as<std::string>();

        if (!fs::is_regular_file(cfg.config_file.value()))
        {
            BOOST_LOG_TRIVIAL(warning) << "User-specified config file does not exist: " << cfg.config_file.value();
        }
    }

    // Apply configuration from the config file before we apply the command line args.
    if (fs::is_regular_file(cfg.config_file.value()))
    {
        std::ifstream config_file_data(cfg.config_file.value(), std::ios::binary);

        try
        {
            const toml::table config_file_tbl = toml::parse(config_file_data);

            if (auto val = config_file_tbl["db"].value<std::string>())
                cfg.db = *val;

            if (auto val = config_file_tbl["http"]["host"].value<std::string>())
                cfg.http_host = *val;

            if (auto val = config_file_tbl["http"]["port"].value<uint16_t>())
                cfg.http_port = *val;

            if (auto val = config_file_tbl["log_level"].value<std::string>())
                cfg.log_level = *val;

            if (auto val = config_file_tbl["session_settings"]["base"].value<std::string>())
            {
                if (*val == "default")               cfg.session_settings = lt::default_settings();
                if (*val == "high_performance_seed") cfg.session_settings = lt::high_performance_seed();
                if (*val == "min_memory_usage")      cfg.session_settings = lt::min_memory_usage();
            }

            if (auto val = config_file_tbl["timer"]["dht_stats"].value<int>())
                cfg.timer_dht_stats = *val;

            if (auto val = config_file_tbl["timer"]["session_stats"].value<int>())
                cfg.timer_session_stats = *val;

            if (auto val = config_file_tbl["timer"]["torrent_updates"].value<int>())
                cfg.timer_torrent_updates = *val;
        }
        catch (const toml::parse_error& err)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse config file '" << cfg.config_file.value() << "': " << err;
        }
    }

    if (vm.count("db"))                    cfg.db                    = vm["db"].as<std::string>();
    if (vm.count("http-host"))             cfg.http_host             = vm["http-host"].as<std::string>();
    if (vm.count("http-port"))             cfg.http_port             = vm["http-port"].as<uint16_t>();
    if (vm.count("log-level"))             cfg.log_level             = vm["log-level"].as<std::string>();
    if (vm.count("session-settings-base"))
    {
        auto val = vm["session-settings-base"].as<std::string>();

        if (val == "default")               cfg.session_settings = lt::default_settings();
        if (val == "high_performance_seed") cfg.session_settings = lt::high_performance_seed();
        if (val == "min_memory_usage")      cfg.session_settings = lt::min_memory_usage();
    }
    if (vm.count("supervised-interval"))   cfg.supervised_interval   = vm["supervised-interval"].as<int>();
    if (vm.count("supervised-pid"))        cfg.supervised_pid        = vm["supervised-pid"].as<pid_t>();
    if (vm.count("timer-dht-stats"))       cfg.timer_dht_stats       = vm["timer-dht-stats"].as<int>();
    if (vm.count("timer-session-stats"))   cfg.timer_session_stats   = vm["timer-session-stats"].as<pid_t>();
    if (vm.count("timer-torrent-updates")) cfg.timer_torrent_updates = vm["timer-torrent-updates"].as<pid_t>();

    return std::move(cfg);
}
