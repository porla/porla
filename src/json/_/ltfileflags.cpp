#include <unordered_set>

#include <libtorrent/file_storage.hpp>

#include "../utils.hpp"

namespace libtorrent
{
    void to_json(nlohmann::json& j, const libtorrent::file_flags_t& flags)
    {
        std::unordered_set<std::string> f;
        if (flags & lt::file_storage::flag_pad_file)   f.insert("pad_file");
        if (flags & lt::file_storage::flag_hidden)     f.insert("hidden");
        if (flags & lt::file_storage::flag_executable) f.insert("executable");
        if (flags & lt::file_storage::flag_symlink)    f.insert("symlink");

        j = f;
    }
}
