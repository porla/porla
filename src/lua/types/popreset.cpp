#include "popreset.hpp"

#include "../../data/models/presets.hpp"
#include "../../query/pql.hpp"

using porla::Lua::Types::PoPreset;

using Preset = porla::Data::Models::Presets::Preset;

void PoPreset::Register(sol::state& lua)
{
    lua.new_usertype<Preset>(
        "PoPreset",
        sol::no_constructor,
        "id", sol::readonly(&Preset::id),
        "name", sol::readonly(&Preset::name),
        "category", sol::readonly(&Preset::category),
        "download_limit", sol::readonly(&Preset::download_limit),
        "flags", sol::readonly(&Preset::flags),
        "flags_mask", sol::readonly(&Preset::flags_mask),
        "max_connections", sol::readonly(&Preset::max_connections),
        "max_uploads", sol::readonly(&Preset::max_uploads),
        // metadata
        "session_id", sol::readonly(&Preset::session_id),
        "save_path", sol::readonly(&Preset::save_path),
        "storage_mode", sol::readonly(&Preset::storage_mode),
        "tags", sol::property([](const Preset& p)
        {
            return sol::as_table(p.tags);
        }),
        "upload_limit", sol::readonly(&Preset::upload_limit)
    );
}
