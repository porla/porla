#pragma once

#include <libtorrent/file_storage.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    struct LtFilenames
    {
        static void Register(sol::state& lua);

        LtFilenames(std::shared_ptr<const lt::file_storage> fs, const lt::renamed_files& renamed);
        LtFilenames(const LtFilenames&) = delete;
        LtFilenames& operator=(const LtFilenames&) = delete;

    private:
        lt::file_index_t FileIndex(int index) const;

        const std::shared_ptr<const lt::file_storage> m_file_storage;
        const lt::renamed_files                       m_renamed_files;
        const lt::filenames                           m_filenames;
    };
}
