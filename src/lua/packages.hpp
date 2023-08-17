#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Packages
{
    class Config
    {
    public:
        static void Register(sol::state& lua);
    };

    class Cron
    {
    public:
        static void Register(sol::state& lua);
    };

    class Events
    {
    public:
        static void Register(sol::state& lua);
    };

    class FileSystem
    {
    public:
        static void Register(sol::state& lua);
    };

    class HttpClient
    {
    public:
        static void Register(sol::state& lua);
    };

    class Json
    {
    public:
        static void Register(sol::state& lua);
    };

    class Log
    {
    public:
        static void Register(sol::state& lua);
    };

    class PQL
    {
    public:
        static void Register(sol::state& lua);
    };

    class Presets
    {
    public:
        static void Register(sol::state& lua);
    };

    class Process
    {
    public:
        static void Register(sol::state& lua);
    };

    class Sessions
    {
    public:
        static void Register(sol::state& lua);
    };

    class Sqlite
    {
    public:
        static void Register(sol::state& lua);
    };

    class Timers
    {
    public:
        static void Register(sol::state& lua);
    };

    class Torrents
    {
    public:
        static void Register(sol::state& lua);
    };

    class Workflows
    {
    public:
        static void Register(sol::state& lua);
    };
}
