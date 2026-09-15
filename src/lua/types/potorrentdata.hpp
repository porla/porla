#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    struct PoTorrentData
    {
        static void Register(sol::state& lua);
    };
}
