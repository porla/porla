#include "../types.hpp"

#include <libtorrent/bencode.hpp>
#include <libtorrent/settings_pack.hpp>

using porla::Lua::Types::LtBencode;

static std::string BencodeLtSettings(const lt::settings_pack& settings)
{
    lt::entry::dictionary_type dict;
    lt::save_settings_to_dict(settings, dict);

    std::string buf;
    lt::bencode(std::back_inserter(buf), dict);

    return buf;
}

void LtBencode::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["bencode"] = sol::overload(
        &BencodeLtSettings
    );
}
