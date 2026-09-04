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
        ("http-webui-file",       po::value<std::string>(), "The path to a zip file with a web UI")
        ("http-webui-repository", po::value<std::string>(), "The name of a GitHub repo (in org/repo format) that has web UI releases")
        ("mmdb-file",             po::value<std::string>(), "The path to a MMDB file that is used in the mmdb.lookup JSONRPC method")
        ("log-level",             po::value<std::string>(), "The minimum log level to print.")
        ("secret-key",            po::value<std::string>(), "The secret key to use when protecting various pieces of data.")
        ("session-settings-base", po::value<std::string>(), "The libtorrent base settings to use")
        ("state-dir",             po::value<std::string>(), "The path to a directory where Porla state will be saved.")
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
