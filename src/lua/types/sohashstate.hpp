#pragma once

#include <memory>

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class SoHashState
    {
    public:
        static void Register(sol::state& lua);
    };
}
