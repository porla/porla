#include "../types.hpp"

#include <libtorrent/storage_defs.hpp>

using porla::Lua::Types::LtStorageMode;

void LtStorageMode::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["storage_mode_t"] = lua.new_usertype<lt::storage_mode_t>(
        "lt.storage_mode_t",
        sol::no_constructor,
        sol::meta_function::equal_to, [](lt::storage_mode_t a, lt::storage_mode_t b) { return a == b; }
    );

    lt["storage_mode_t"]["allocate"] = sol::var(lt::storage_mode_t::storage_mode_allocate);
    lt["storage_mode_t"]["sparse"] = sol::var(lt::storage_mode_t::storage_mode_sparse);
}
