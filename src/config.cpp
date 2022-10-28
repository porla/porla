#include "config.hpp"

#include <filesystem>

#include <boost/program_options.hpp>

namespace fs = std::filesystem;
namespace po = boost::program_options;

using porla::Config;

Config Config::Load(int argc, char **argv)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("config",         po::value<std::string>(), "Path to a porla.toml config file")
        ("supervised-pid", po::value<pid_t>(),       "A pid to a parent process. If this pid dies, we shut down.")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    auto supervised_pid = vm.count("supervised-pid")
        ? std::optional(vm["supervised-pid"].as<pid_t>())
        : std::nullopt;

    // Config load order
    // 1. CLI args
    // 2. Env PORLA_CONFIG
    // 3. Env PORLA_CONFIG_FILE
    // 4. Current working directory

    // Load config file from CLI args first, if if exist
    if (vm.count("config"))
    {
        std::ifstream cfg(vm["config"].as<std::string>(), std::ios::binary);
        return {
            .supervised_pid = supervised_pid,
            .tbl            = toml::parse(cfg)
        };
    }

    if (const char* porlaConfig = std::getenv("PORLA_CONFIG"))
    {
        return {
            .supervised_pid = supervised_pid,
            .tbl            = toml::parse(porlaConfig)
        };
    }

    if (const char* porlaConfigFile = std::getenv("PORLA_CONFIG_FILE"))
    {
        std::ifstream cfg(porlaConfigFile, std::ios::binary);
        return {
            .supervised_pid = supervised_pid,
            .tbl            = toml::parse(cfg)
        };
    }

    fs::path p = fs::current_path() / "porla.toml";

    if (fs::exists(p) && fs::is_regular_file(p))
    {
        std::ifstream cfg(p, std::ios::binary);
        return {
            .supervised_pid = supervised_pid,
            .tbl            = toml::parse(cfg)
        };
    }

    return {
        .supervised_pid = supervised_pid,
        .tbl            = {}
    };
}
