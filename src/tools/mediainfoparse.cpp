#include "mediainfoparse.hpp"

#include <iostream>

#include <boost/program_options.hpp>

#include "../config.hpp"
#include "../mediainfo/parser.hpp"

namespace po = boost::program_options;

int porla::Tools::MediaInfoParse(int argc, char **argv, std::unique_ptr<porla::Config> cfg)
{
    po::options_description opts_desc;
    opts_desc.add_options()
        ("cmd", "")
        ("input", "Media file to parse");

    po::positional_options_description pos_desc;
    pos_desc.add("cmd", 1);
    pos_desc.add("input", 1);

    po::variables_map vm;

    try
    {
        auto parsed = po::command_line_parser(argc, argv)
            .options(opts_desc)
            .positional(pos_desc)
            .allow_unregistered()
            .run();

        po::store(parsed, vm);
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    const auto input_file = vm["input"].as<std::string>();

    porla::MediaInfo::Parser::Parse(input_file);

    return 0;
}
