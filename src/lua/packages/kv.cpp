#include "kv.hpp"

#include "../../data/models/keyvaluestore.hpp"

#include "../pluginstate.hpp"
#include "../types/pojson.hpp"

using porla::Lua::Packages::Kv;

sol::object Kv::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("get", [](sol::this_state ts, const std::string& key) -> std::optional<sol::object>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return std::nullopt; }

        const auto value = Data::Models::KeyValueStore::Get(state->db, key);

        if (value.is_null())
        {
            return sol::lua_nil;
        }

        return Types::PoJson::ToLua(ts, value, 0);
    });

    tbl.set_function("set", [](sol::this_state ts, const std::string& key, sol::object value)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return; }

        const auto json_value = Types::PoJson::ToJson(ts, value, 0);

        Data::Models::KeyValueStore::Set(state->db, key, json_value);
    });

    return tbl;
}
