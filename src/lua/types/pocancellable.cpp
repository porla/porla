#include "pocancellable.hpp"

using porla::Lua::Types::PoCancellable;

void PoCancellable::Register(sol::state& lua)
{
    lua.new_usertype<PoCancellable>(
        "PoCancellable",
        sol::no_constructor,
        "cancel", &PoCancellable::Cancel);
}
