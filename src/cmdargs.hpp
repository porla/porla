#pragma once

#include <boost/program_options.hpp>

namespace porla
{
    class CmdArgs
    {
    public:
        static int Help();
        static boost::program_options::variables_map Parse(int argc, char* argv[]);
    };
}
