#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

using porla::Lua::Types::LtOpenFileState;

namespace
{
    std::vector<std::string> OpenModeNames(const lt::file_open_mode_t mode)
    {
        namespace fom = lt::file_open_mode;

        std::vector<std::string> names;

        // the two low bits are the read/write mode (an enum, read_only == 0), not flags
        const auto rw = mode & fom::rw_mask;

        if      (rw == fom::read_write) names.emplace_back("read_write");
        else if (rw == fom::write_only) names.emplace_back("write_only");
        else                            names.emplace_back("read_only");

        if (mode & fom::sparse)        names.emplace_back("sparse");
        if (mode & fom::no_atime)      names.emplace_back("no_atime");
        if (mode & fom::random_access) names.emplace_back("random_access");
        if (mode & fom::mmapped)       names.emplace_back("mmapped");

        return names;
    }
}

void LtOpenFileState::Register(sol::state& lua)
{
    lua.new_usertype<lt::open_file_state>(
        "LtOpenFileState",
        sol::no_constructor,
        "file_index", sol::readonly_property([](const lt::open_file_state& ofs) { return static_cast<int>(ofs.file_index); }),
        "open_mode",  sol::readonly_property([](const lt::open_file_state& ofs) { return sol::as_table(OpenModeNames(ofs.open_mode)); }),
        "last_use",   sol::readonly_property([](const lt::open_file_state& ofs) { return ofs.last_use.time_since_epoch().count(); }));
}
