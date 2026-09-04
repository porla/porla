#include "sessions.hpp"

#include "../pluginstate.hpp"
#include "../../data/models/sessions.hpp"

sol::object porla::Lua::Packages::Sessions::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    return lua.create_table_with(
        "count", [](sol::this_state ts) -> sol::object
        {
            sol::state_view lua(ts);

            auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
            auto state = weak.lock();

            if (state == nullptr)
            {
                return sol::nil;
            }

            const auto all_sessions    = porla::Data::Models::Sessions::List(state->db);
            const auto loaded_sessions = std::count_if(
                all_sessions.begin(),
                all_sessions.end(),
                [state](const auto s) { return state->sessions.Get(s.id) != nullptr; });

            return sol::make_object(lua, loaded_sessions);
        },
        "default", [](sol::this_state ts)
        {
            return sol::nil;
        },
        "get", sol::overload(
            [](sol::this_state ts, int id)
            {
                return "by id";
            },
            [](sol::this_state ts, const std::string name)
            {
                return "by name";
            })
        );
}
