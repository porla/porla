#pragma once

#include <boost/program_options.hpp>

namespace porla
{
    class Logger
    {
    public:
        static void Setup(const boost::program_options::variables_map& cmd) noexcept;
    };
}