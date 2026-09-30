#include "../types.hpp"

#include <libtorrent/load_torrent.hpp>
#include <libtorrent/torrent_info.hpp>

using porla::Lua::Types::LtTorrentInfo;

void LtTorrentInfo::Register(sol::state& lua)
{
    lua.new_usertype<lt::torrent_info>(
        "LtTorrentInfo",
        sol::no_constructor,
        "info_hash",   &lt::torrent_info::info_hashes,
        "layout", [](const std::shared_ptr<const lt::torrent_info>& ti)
        {
            if (!ti->is_valid())
            {
                throw sol::error("torrent has no metadata");
            }

            return std::shared_ptr<const lt::file_storage>(ti, &ti->layout());
        },
        "name",        &lt::torrent_info::name,
        "num_files",   &lt::torrent_info::num_files,
        "num_pieces",  &lt::torrent_info::num_pieces,
        "priv",        &lt::torrent_info::priv,
        "total_size",  &lt::torrent_info::total_size);
}
