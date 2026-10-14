#pragma once

#include <sol/sol.hpp>

#define PORLA_LUA_PACKAGE(name) \
    struct name { static sol::object Load(sol::this_state s); };

namespace porla::Lua::Packages
{
    PORLA_LUA_PACKAGE(Codec)
    PORLA_LUA_PACKAGE(Crypto)
    PORLA_LUA_PACKAGE(Events)
    PORLA_LUA_PACKAGE(Filesystem)
    PORLA_LUA_PACKAGE(HttpClient)
    PORLA_LUA_PACKAGE(HttpServer)
    PORLA_LUA_PACKAGE(JsonRpc)
    PORLA_LUA_PACKAGE(Kv)
    PORLA_LUA_PACKAGE(Presets)
    PORLA_LUA_PACKAGE(Runtime)
    PORLA_LUA_PACKAGE(Sessions)
    PORLA_LUA_PACKAGE(Sockets)
    PORLA_LUA_PACKAGE(Timers)
    PORLA_LUA_PACKAGE(Zip)
}

#undef PORLA_LUA_PACKAGE
