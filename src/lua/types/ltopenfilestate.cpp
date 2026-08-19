#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

using porla::Lua::Types::LtOpenFileState;

void LtOpenFileState::Register(sol::state& lua)
{
    lua.new_usertype<lt::open_file_state>(
        "lt.open_file_state",
        sol::no_constructor,
        "file_index", &lt::open_file_state::file_index,
        "open_mode",  &lt::open_file_state::open_mode,
        "last_use",   sol::readonly_property([](const lt::open_file_state& ofs) { return ofs.last_use.time_since_epoch().count(); }));
}
