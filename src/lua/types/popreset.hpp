#pragma once

#include <memory>

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class PoPreset
    {
    public:
        static void Register(sol::state& lua);
    };
}
