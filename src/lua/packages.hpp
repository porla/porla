#pragma once

#include <sol/sol.hpp>

#define DECLARE_PACKAGE(name) \
    class name                \
    {                         \
    public:                   \
        static void Register(sol::state& lua); \
    };

namespace porla::Lua::Packages
{
    DECLARE_PACKAGE(Config)
    DECLARE_PACKAGE(Cron)
    DECLARE_PACKAGE(Events)
    DECLARE_PACKAGE(FileSystem)
    DECLARE_PACKAGE(HttpClient)
    DECLARE_PACKAGE(Json)
    DECLARE_PACKAGE(Log)
    DECLARE_PACKAGE(PQL)
    DECLARE_PACKAGE(Presets)
    DECLARE_PACKAGE(Process)
    DECLARE_PACKAGE(Sessions)
    DECLARE_PACKAGE(Sqlite)
    DECLARE_PACKAGE(Timers)
    DECLARE_PACKAGE(Torrents)
    DECLARE_PACKAGE(Workflows)
}
