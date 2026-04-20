#include "cmdargs.hpp"

#include <iostream>

namespace po = boost::program_options;
using porla::CmdArgs;

static po::options_description Options()
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("bootstrap-file",         po::value<std::string>(), "Path to a bootstrap.lua file.")
        ("config-file",            po::value<std::string>(), "Path to a porla.toml config file.")
        ("core-dir",               po::value<std::string>(), "Path to a directory where Porla core will be loaded from.")
        ("core-zip",               po::value<std::string>(), "Path to a zipball where Porla core will be loaded from.")
        ("core-github-repository", po::value<std::string>(), "The repository to a GitHub repository in owner/repo format.")
        ("core-github-release",    po::value<std::string>(), "The version of a GitHub release in the core repository to use.")
        ("help",                                             "Show usage")
        ("log-level",              po::value<std::string>(), "The minimum log level to print.")
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
    const auto options = Options();
    const auto parsed = po::command_line_parser(argc, argv)
        .options(options)
        .allow_unregistered()
        .run();

    po::variables_map vm;
    po::store(parsed, vm);
    po::notify(vm);

    return vm;
}
