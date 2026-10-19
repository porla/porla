#pragma once

#include <functional>

#include <sol/sol.hpp>

namespace porla::Lua
{
    struct LuaState;
}

namespace porla::Lua::Types
{
    struct PoCancellable
    {
        static void Register(sol::state& lua);

        explicit PoCancellable(std::function<void(LuaState&)> cancel);
        void Cancel(sol::this_state ts);

    private:
        std::function<void(LuaState&)> m_cancel;
    };
}
