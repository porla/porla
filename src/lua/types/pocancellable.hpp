#pragma once

#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    struct PoCancellable
    {
        static void Register(sol::state& lua);

        virtual void Cancel(sol::this_state ts) = 0;
    };
}
