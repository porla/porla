#include "../packages.hpp"

#include "../../data/models/presets.hpp"
#include "../plugin.hpp"
#include "../registry.hpp"

using porla::Lua::Packages::Presets;

void Presets::Register(sol::state& lua)
{
    auto preset_type = lua.new_usertype<porla::Data::Models::Presets::Preset>(
        "porla.Preset",
        sol::no_constructor,
        "category",        sol::readonly(&porla::Data::Models::Presets::Preset::category),
        "download_limit",  sol::readonly(&porla::Data::Models::Presets::Preset::download_limit),
        "max_connections", sol::readonly(&porla::Data::Models::Presets::Preset::max_connections),
        "max_uploads",     sol::readonly(&porla::Data::Models::Presets::Preset::max_uploads),
        "save_path",       sol::readonly(&porla::Data::Models::Presets::Preset::save_path),
        "session_id",      sol::readonly(&porla::Data::Models::Presets::Preset::session_id),
        "tags",            sol::readonly(&porla::Data::Models::Presets::Preset::tags),
        "upload_limit",    sol::readonly(&porla::Data::Models::Presets::Preset::upload_limit));

    lua["package"]["preload"]["presets"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table presets = lua.create_table();

        presets["get"] = [](sol::this_state s, const std::string& name) -> std::optional<porla::Data::Models::Presets::Preset>
        {
            sol::state_view lua{s};

            return porla::Data::Models::Presets::GetByName(
                lua.registry()["db"].get<porla::Lua::Registry::Sqlite3>().db,
                name);
        };

        return presets;
    };
}
