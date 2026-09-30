#include "ltfilenames.hpp"

#include <libtorrent/file_storage.hpp>

using porla::Lua::Types::LtFilenames;

void LtFilenames::Register(sol::state& lua)
{
    lua.new_usertype<LtFilenames>(
        "LtFilenames",
        sol::call_constructor,
        sol::factories([](const std::shared_ptr<const lt::file_storage>& storage, const lt::renamed_files& renamed)
        {
            return std::make_shared<LtFilenames>(storage, renamed);
        }),
        "file_absolute_path", [](const LtFilenames& fn, int file_index)
        {
            return fn.m_filenames.file_absolute_path(fn.FileIndex(file_index));
        },
        "file_path", sol::overload(
            [](const LtFilenames& fn, int file_index)
            {
                return fn.m_filenames.file_path(fn.FileIndex(file_index));
            },
            [](const LtFilenames& fn, int file_index, const std::string& save_path)
            {
                return fn.m_filenames.file_path(fn.FileIndex(file_index), save_path);
            }),
        "num_files", [](const LtFilenames& fn) { return fn.m_filenames.num_files(); },
        "num_pieces", [](const LtFilenames& fn) { return fn.m_filenames.num_pieces(); }
    );
}

LtFilenames::LtFilenames(std::shared_ptr<const lt::file_storage> file_storage, const lt::renamed_files& renamed)
    : m_file_storage(std::move(file_storage))
    , m_renamed_files(renamed)
    , m_filenames(*m_file_storage, m_renamed_files)
{
}

lt::file_index_t LtFilenames::FileIndex(int index) const
{
    const auto num_files = m_filenames.num_files();

    if (index < 0 || index >= num_files)
    {
        throw sol::error("file index out of range");
    }

    return lt::file_index_t(index);
}
