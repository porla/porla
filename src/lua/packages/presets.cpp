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
        "is_default",      sol::readonly(&porla::Data::Models::Presets::Preset::is_default),
        "max_connections", sol::readonly(&porla::Data::Models::Presets::Preset::max_connections),
        "max_uploads",     sol::readonly(&porla::Data::Models::Presets::Preset::max_uploads),
        "name",            sol::readonly(&porla::Data::Models::Presets::Preset::name),
        "save_path",       sol::readonly(&porla::Data::Models::Presets::Preset::save_path),
        "session_id",      sol::readonly(&porla::Data::Models::Presets::Preset::session_id),
        "tags",            sol::readonly(&porla::Data::Models::Presets::Preset::tags),
        "upload_limit",    sol::readonly(&porla::Data::Models::Presets::Preset::upload_limit));

    lua["package"]["preload"]["presets"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table presets = lua.create_table();

        presets["default"] = [](sol::this_state s) -> std::optional<porla::Data::Models::Presets::Preset>
        {
            sol::state_view lua{s};

            return porla::Data::Models::Presets::GetDefault(
                lua.registry()["db"].get<porla::Lua::Registry::Sqlite3>().db);
        };

        presets["get"] = [](sol::this_state s, int id) -> std::optional<porla::Data::Models::Presets::Preset>
        {
            sol::state_view lua{s};

            return porla::Data::Models::Presets::GetById(
                lua.registry()["db"].get<porla::Lua::Registry::Sqlite3>().db,
                id);
        };

        return presets;
    };
}
