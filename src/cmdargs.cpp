#include "cmdargs.hpp"

#include <iostream>

namespace po = boost::program_options;
using porla::CmdArgs;

static po::options_description Options()
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("config-file",           po::value<std::string>(), "Path to a porla.toml config file.")
        ("db",                    po::value<std::string>(), "Path to where the database will be stored.")
        ("help",                                            "Show usage")
        ("http-base-path",        po::value<std::string>(), "The base path for HTTP routes")
        ("http-host",             po::value<std::string>(), "The host to listen on for HTTP traffic.")
        ("http-metrics-enabled",  po::value<bool>(),        "Set to true if the metrics endpoint should be enabled")
        ("http-port",             po::value<uint16_t>(),    "The port to listen on for HTTP traffic.")
        ("http-webui-enabled",    po::value<bool>(),        "Set to true if the web UI should be enabled")
        ("log-level",             po::value<std::string>(), "The minimum log level to print.")
        ("secret-key",            po::value<std::string>(), "The secret key to use when protecting various pieces of data.")
        ("session-settings-base", po::value<std::string>(), "The libtorrent base settings to use")
        ("state-dir",             po::value<std::string>(), "The path to a directory where Porla state will be saved.")
        ("supervised-interval",   po::value<int>(),         "The interval to use when checking the supervisor pid.")
        ("supervised-pid",        po::value<pid_t>(),       "A pid to a parent process. If this pid dies, we shut down.")
        ("timer-dht-stats",       po::value<int>(),         "The interval to use for the DHT stats updates.")
        ("timer-session-stats",   po::value<int>(),         "The interval to use for the session stats updates.")
        ("timer-torrent-updates", po::value<int>(),         "The interval to use for the torrent updates.")
        ("workflow-dir",          po::value<std::string>(), "The directory where workflow files are stored.")
        ;

    return desc;
}

int CmdArgs::Help()
{
    std::cout << Options();
    return 0;
}

boost::program_options::variables_map CmdArgs::Parse(int argc, char **argv)
{
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, Options()), vm);
    po::notify(vm);

    return vm;
}
