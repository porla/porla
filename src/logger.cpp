#include "logger.hpp"

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

using porla::Logger;

void Logger::Setup(const boost::program_options::variables_map& cmd) noexcept
{
    std::string configured_level;

    if (auto env_level = std::getenv("PORLA_LOG_LEVEL"))
    {
        configured_level = env_level;
    }

    boost::log::trivial::severity_level log_level = boost::log::trivial::info;

    if (configured_level == "trace")   { log_level = boost::log::trivial::severity_level::trace; }
    if (configured_level == "debug")   { log_level = boost::log::trivial::severity_level::debug; }
    if (configured_level == "info")    { log_level = boost::log::trivial::severity_level::info; }
    if (configured_level == "warning") { log_level = boost::log::trivial::severity_level::warning; }
    if (configured_level == "error")   { log_level = boost::log::trivial::severity_level::error; }
    if (configured_level == "fatal")   { log_level = boost::log::trivial::severity_level::fatal; }

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= log_level);
}
