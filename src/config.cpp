#include "config.hpp"

#include <filesystem>

#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

using porla::Config;

Config Config::Load(int argc, char **argv)
{
    Config cfg = {};

    if (auto val = std::getenv("PORLA_DB"))                    cfg.db = val;
    if (auto val = std::getenv("PORLA_HTTP_HOST"))             cfg.http_host = val;
    if (auto val = std::getenv("PORLA_HTTP_PORT"))             cfg.http_port = std::stoi(val);
    if (auto val = std::getenv("PORLA_LOG_LEVEL"))             cfg.log_level = val;
    if (auto val = std::getenv("PORLA_SUPERVISED_INTERVAL"))   cfg.supervised_interval = std::stoi(val);
    if (auto val = std::getenv("PORLA_SUPERVISED_PID"))        cfg.supervised_pid = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_DHT_STATS"))       cfg.timer_dht_stats = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_SESSION_STATS"))   cfg.timer_session_stats = std::stoi(val);
    if (auto val = std::getenv("PORLA_TIMER_TORRENT_UPDATES")) cfg.timer_torrent_updates = std::stoi(val);

    po::options_description desc("Allowed options");
    desc.add_options()
        ("config",                po::value<std::string>(), "Path to a porla.toml config file.")
        ("db",                    po::value<std::string>(), "Path to where the database will be stored.")
        ("http-host",             po::value<std::string>(), "The host to listen on for HTTP traffic.")
        ("http-port",             po::value<uint16_t>(),    "The port to listen on for HTTP traffic.")
        ("log-level",             po::value<std::string>(), "The minimum log level to print.")
        ("supervised-interval",   po::value<int>(),         "The interval to use when checking the supervisor pid.")
        ("supervised-pid",        po::value<pid_t>(),       "A pid to a parent process. If this pid dies, we shut down.")
        ("timer-dht-stats",       po::value<int>(),         "The interval to use for the DHT stats updates.")
        ("timer-session-stats",   po::value<int>(),         "The interval to use for the session stats updates.")
        ("timer-torrent-updates", po::value<int>(),         "The interval to use for the torrent updates.")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("db"))                    cfg.db                    = vm["db"].as<std::string>();
    if (vm.count("http-host"))             cfg.http_host             = vm["http-host"].as<std::string>();
    if (vm.count("http-port"))             cfg.http_port             = vm["http-port"].as<uint16_t>();
    if (vm.count("log-level"))             cfg.log_level             = vm["log-level"].as<std::string>();
    if (vm.count("supervised-interval"))   cfg.supervised_interval   = vm["supervised-interval"].as<int>();
    if (vm.count("supervised-pid"))        cfg.supervised_pid        = vm["supervised-pid"].as<pid_t>();
    if (vm.count("timer-dht-stats"))       cfg.timer_dht_stats       = vm["timer-dht-stats"].as<int>();
    if (vm.count("timer-session-stats"))   cfg.timer_session_stats   = vm["timer-session-stats"].as<pid_t>();
    if (vm.count("timer-torrent-updates")) cfg.timer_torrent_updates = vm["timer-torrent-updates"].as<pid_t>();

    std::vector<fs::path> config_search_paths =
    {
        fs::current_path() / "porla.toml"
    };

    // TODO: search for files in order, apply first.

    return std::move(cfg);
}
