#include "../packages.hpp"

#include "../../config.hpp"
#include "../plugin.hpp"

using porla::Lua::Packages::Presets;

void Presets::Register(sol::state& lua)
{
    auto preset_type = lua.new_usertype<porla::Config::Preset>(
        "porla.Preset",
        sol::no_constructor,
        "category",        sol::readonly(&porla::Config::Preset::category),
        "download_limit",  sol::readonly(&porla::Config::Preset::download_limit),
        "max_connections", sol::readonly(&porla::Config::Preset::max_connections),
        "max_uploads",     sol::readonly(&porla::Config::Preset::max_uploads),
        "save_path",       sol::readonly(&porla::Config::Preset::save_path),
        "session",         sol::readonly(&porla::Config::Preset::session),
        "tags",            sol::readonly(&porla::Config::Preset::tags),
        "upload_limit",    sol::readonly(&porla::Config::Preset::upload_limit));

    lua["package"]["preload"]["presets"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table presets = lua.create_table();

        presets["get"] = [](sol::this_state s, const std::string& name) -> std::optional<porla::Config::Preset>
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

            if (options.config.presets.find(name) == options.config.presets.end())
            {
                return std::nullopt;
            }

            return options.config.presets.at(name);
        };

        return presets;
    };
}
