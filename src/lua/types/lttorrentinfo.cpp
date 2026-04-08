#include "../types.hpp"

#include <libtorrent/torrent_info.hpp>

using porla::Lua::Types::LtTorrentInfo;

void LtTorrentInfo::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["torrent_info"] = lua.new_usertype<lt::torrent_info>(
        "lt.torrent_info",
        sol::no_constructor,

        "from_buffer", sol::factories([](const std::string& data) -> std::pair<std::shared_ptr<lt::torrent_info>, std::optional<std::string>>
                       {
                           lt::error_code ec;
                           auto ti = std::make_shared<lt::torrent_info>(data, ec, lt::from_span);
                           if (ec) return std::pair(nullptr, ec.message());
                           return std::pair(ti, std::nullopt);
                       }),

        "from_file",   sol::factories([](const std::string& data) -> std::pair<std::shared_ptr<lt::torrent_info>, std::optional<std::string>>
                       {
                           lt::error_code ec;
                           auto ti = std::make_shared<lt::torrent_info>(data, ec);
                           if (ec) return std::pair(nullptr, ec.message());
                           return std::pair(ti, std::nullopt);
                       }),

        "comment",     &lt::torrent_info::comment,
        "creator",     &lt::torrent_info::creator,
        "files",       &lt::torrent_info::files,
        "info_hash",   &lt::torrent_info::info_hashes,
        "name",        &lt::torrent_info::name,
        "num_files",   &lt::torrent_info::num_files,
        "num_pieces",  &lt::torrent_info::num_pieces,
        "priv",        &lt::torrent_info::priv,
        "trackers",    &lt::torrent_info::trackers,
        "total_size",  &lt::torrent_info::total_size);
}
