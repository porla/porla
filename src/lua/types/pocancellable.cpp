#include "pocancellable.hpp"

#include <utility>

#include "../pluginstate.hpp"

using porla::Lua::Types::PoCancellable;

void PoCancellable::Register(sol::state& lua)
{
    lua.new_usertype<PoCancellable>(
        "PoCancellable",
        sol::no_constructor,
        "cancel", &PoCancellable::Cancel);
}

PoCancellable::PoCancellable(std::function<void(LuaState&)> cancel)
    : m_cancel(std::move(cancel))
{
}

void PoCancellable::Cancel(sol::this_state ts)
{
    auto state = LuaState::From(ts);
    if (state == nullptr || !m_cancel) { return; }

    std::exchange(m_cancel, nullptr)(*state);
}
