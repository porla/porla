#include "../types.hpp"

#include <libtorrent/file_storage.hpp>

using porla::Lua::Types::LtFileStorage;

void LtFileStorage::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["file_storage"] = lua.new_usertype<lt::file_storage>(
        "lt.file_storage",
        sol::no_constructor,
        "file_name", [](const lt::file_storage& fs, int index) { return fs.file_name(lt::file_index_t{index}).to_string(); },
        "file_path", [](const lt::file_storage& fs, int index) { return fs.file_path(lt::file_index_t{index}); },
        "file_size", [](const lt::file_storage& fs, int index) { return fs.file_size(lt::file_index_t{index}); });
}
