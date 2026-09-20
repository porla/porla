#include "presets.hpp"

#include "../../data/models/presets.hpp"
#include "../pluginstate.hpp"

using porla::Lua::Packages::Presets;

sol::object Presets::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("default", [](sol::this_state ts) -> std::optional<Data::Models::Presets::Preset>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return std::nullopt; }

        return Data::Models::Presets::GetDefault(state->db);
    });

    tbl.set_function("get", sol::overload(
        [](sol::this_state ts, int id) -> std::optional<Data::Models::Presets::Preset>
        {
            sol::state_view lua(ts);

            auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
            auto state = weak.lock();

            if (state == nullptr) { return std::nullopt; }

            return Data::Models::Presets::GetById(state->db, id);
        },
        [](sol::this_state ts, const std::string& name) -> std::optional<Data::Models::Presets::Preset>
        {
            sol::state_view lua(ts);

            auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
            auto state = weak.lock();

            if (state == nullptr) { return std::nullopt; }

            return Data::Models::Presets::GetByName(state->db, name);
        })
    );

    return tbl;
}
