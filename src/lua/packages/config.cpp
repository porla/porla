#include "../packages.hpp"

#include <boost/log/trivial.hpp>

#include "../plugin.hpp"
#include "../../config.hpp"

using porla::Lua::Packages::Config;

void Config::Register(sol::state& lua)
{
    lua["package"]["preload"]["config.app"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        const auto& options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

        sol::table t = lua.create_table();
        t["db_file"] = options.config.db_file;
        return t;
    };

    lua["package"]["preload"]["config"] = [](sol::this_state s) -> sol::object
    {
        sol::state_view lua{s};
        const auto& options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

        /*if (options.plugin_config.has_value())
        {
            const fs::path actual_state_dir = options.config.state_dir.value_or(fs::current_path());

            sol::environment env(lua, sol::create);
            env["__state_dir"] = fs::absolute(actual_state_dir).string();

            try
            {
                return lua.script(*options.plugin_config, env);
            }
            catch (const sol::error& err)
            {
                BOOST_LOG_TRIVIAL(error) << "Plugin configuration error: " << err.what();
                throw;
            }
        }*/

        return sol::nil;
    };
}
