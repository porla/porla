#include "../types.hpp"

#include <libtorrent/file_storage.hpp>

using porla::Lua::Types::LtFileStorage;

namespace
{
    lt::file_index_t FileIndex(const lt::file_storage& fs, int index)
    {
        const auto num_files = fs.num_files();

        if (index < 0 || index >= num_files)
        {
            throw sol::error("file index out of range");
        }

        return lt::file_index_t(index);
    }
}

void LtFileStorage::Register(sol::state& lua)
{
    lua.new_usertype<lt::file_storage>(
        "LtFileStorage",
        sol::no_constructor,
        "file_name", [](const lt::file_storage& fs, int index) { return fs.file_name(FileIndex(fs, index)); },
        "file_path", [](const lt::file_storage& fs, int index) { return fs.file_path(FileIndex(fs, index)); },
        "file_size", [](const lt::file_storage& fs, int index) { return fs.file_size(FileIndex(fs, index)); }
    );
}
