#pragma once

#include <libtorrent/error_code.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class PoError
    {
    public:
        static sol::object Construct(sol::this_state ts, const lt::error_code& ec);
    };
}
