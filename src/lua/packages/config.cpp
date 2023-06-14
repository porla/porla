#include "config.hpp"

#include <toml++/toml.h>

#include "../plugins/plugin.hpp"
#include "../../config.hpp"

using porla::Lua::Packages::Config;

static sol::object TomlNode2LuaObject(sol::state_view& lua, const toml::node& node)
{
    switch (node.type())
    {
        case toml::node_type::none:
            return {};

        case toml::node_type::table:
        {
            sol::table tbl = lua.create_table();

            for (const auto& [ key, value ] : *node.as_table())
            {
                tbl[key.str()] = TomlNode2LuaObject(lua, value);
            }

            return tbl;
        }

        case toml::node_type::array:
        {
            sol::table tbl = lua.create_table();
            const auto& arr = *node.as_array();

            for (int i = 0; i < arr.size(); i++)
            {
                // Lua arrays start at 1.
                tbl[i + 1] = TomlNode2LuaObject(lua, arr[i]);
            }

            return tbl;
        }

        case toml::node_type::string:
            return sol::make_object(lua, node.as_string()->get());

        case toml::node_type::integer:
            return sol::make_object(lua, node.as_integer()->get());

        case toml::node_type::floating_point:
            return sol::make_object(lua, node.as_floating_point()->get());

        case toml::node_type::boolean:
            return sol::make_object(lua, node.as_boolean()->get());

        case toml::node_type::date:
            break;
        case toml::node_type::time:
            break;
        case toml::node_type::date_time:
            break;
    }

    return {};
}

void Config::Register(sol::state& lua)
{
    lua["package"]["preload"]["config"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        const auto& options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();
        return TomlNode2LuaObject(lua, options.config.config_tbl);
    };
}
