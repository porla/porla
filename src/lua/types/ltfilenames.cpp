#include "ltfilenames.hpp"

#include <libtorrent/file_storage.hpp>

using porla::Lua::Types::LtFilenames;

void LtFilenames::Register(sol::state& lua)
{
    lua.new_usertype<lt::filenames>(
        "LtFilenames",
        sol::call_constructor,
        sol::factories([](const lt::file_storage& storage, const lt::renamed_files& renamed)
        {
            return lt::filenames(storage, renamed);
        }),
        "file_absolute_path", [](const lt::filenames& fn, int file_index)
        {
            return fn.file_absolute_path(lt::file_index_t{file_index});
        },
        "file_path", sol::overload(
            [](const lt::filenames& fn, int file_index)
            {
                return fn.file_path(lt::file_index_t{file_index});
            },
            [](const lt::filenames& fn, int file_index, const std::string& save_path)
            {
                return fn.file_path(lt::file_index_t{file_index}, save_path);
            }),
        "num_files", &lt::filenames::num_files,
        "num_pieces", &lt::filenames::num_pieces
    );
}
