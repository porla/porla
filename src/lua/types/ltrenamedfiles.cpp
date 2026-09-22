#include "ltrenamedfiles.hpp"

#include <libtorrent/file_storage.hpp>

using porla::Lua::Types::LtRenamedFiles;

void LtRenamedFiles::Register(sol::state& lua)
{
    lua.new_usertype<lt::renamed_files>(
        "LtRenamedFiles",
        sol::no_constructor);
}
