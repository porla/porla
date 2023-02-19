#pragma once

#include <libtorrent/fwd.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    struct LibtorrentTorrentHandle
    {
        static void Register(sol::state& lua);
    };
}
