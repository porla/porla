#include "../types.hpp"

#include <toml++/toml.hpp>

using porla::Lua::Types::Toml;

sol::table toml_table_to_lua(sol::state_view lua, const toml::table& tbl);
sol::object toml_node_to_lua(sol::state_view lua, const toml::node& node);

void Toml::Register(sol::state& lua)
{
    sol::table toml = lua["toml"].valid()
        ? lua["toml"].get<sol::table>()
        : lua.create_named_table("toml");

    toml["parse"] = [](sol::this_state ts, const std::string& buffer)
    {
        sol::state_view lua(ts);
        return toml_table_to_lua(lua, toml::parse(buffer));
    };
}

sol::object toml_node_to_lua(sol::state_view lua, const toml::node& node)
{
    switch (node.type())
    {
        case toml::node_type::string:         return sol::make_object(lua, *node.as_string());
        case toml::node_type::integer:        return sol::make_object(lua, *node.as_integer());
        case toml::node_type::floating_point: return sol::make_object(lua, *node.as_floating_point());
        case toml::node_type::boolean:        return sol::make_object(lua, *node.as_boolean());
        case toml::node_type::table:          return sol::make_object(lua, toml_table_to_lua(lua, *node.as_table()));
        case toml::node_type::array:
        {
            sol::table arr = lua.create_table();
            int i = 1;

            for (const auto& elem : *node.as_array())
            {
                arr[i++] = toml_node_to_lua(lua, elem);
            }

            return arr;
        }
        default: return sol::nil;
    }
}

sol::table toml_table_to_lua(sol::state_view lua, const toml::table& tbl)
{
    sol::table t = lua.create_table();

    for (const auto& [k, v] : tbl)
    {
        t[k.str()] = toml_node_to_lua(lua, v);
    }

    return t;
}
