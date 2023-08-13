#include "presets.hpp"

#include "../../config.hpp"
#include "../plugins/plugin.hpp"

using porla::Lua::Packages::Presets;

void Presets::Register(sol::state& lua)
{
    auto preset_type = lua.new_usertype<Config::Preset>(
        "porla.Preset",
        sol::no_constructor,
        "category",        sol::readonly(&Config::Preset::category),
        "download_limit",  sol::readonly(&Config::Preset::download_limit),
        "max_connections", sol::readonly(&Config::Preset::max_connections),
        "max_uploads",     sol::readonly(&Config::Preset::max_uploads),
        "save_path",       sol::readonly(&Config::Preset::save_path),
        "session",         sol::readonly(&Config::Preset::session),
        "tags",            sol::readonly(&Config::Preset::tags),
        "upload_limit",    sol::readonly(&Config::Preset::upload_limit));

    lua["package"]["preload"]["presets"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table presets = lua.create_table();

        presets["get"] = [](sol::this_state s, const std::string& name) -> std::optional<Config::Preset>
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            if (options.config.presets.find(name) == options.config.presets.end())
            {
                return std::nullopt;
            }

            return options.config.presets.at(name);
        };

        return presets;
    };
}
