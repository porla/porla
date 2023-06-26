#include "log.hpp"

#include <boost/log/trivial.hpp>

using porla::Lua::Packages::Log;

void Log::Register(sol::state& lua)
{
    lua["package"]["preload"]["log"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table log = lua.create_table();

        log["debug"]   = [](const std::string& message) { BOOST_LOG_TRIVIAL(debug) << message; };
        log["error"]   = [](const std::string& message) { BOOST_LOG_TRIVIAL(error) << message; };
        log["info"]    = [](const std::string& message) { BOOST_LOG_TRIVIAL(info) << message; };
        log["warning"] = [](const std::string& message) { BOOST_LOG_TRIVIAL(warning) << message; };

        return log;
    };
}
