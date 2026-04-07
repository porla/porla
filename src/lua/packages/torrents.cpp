#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../plugin.hpp"
#include "../../config.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"
#include "../../utils/ratio.hpp"

using porla::Lua::Packages::Torrents;

void Torrents::Register(sol::state& lua)
{
    auto announce_endpoint_type = lua.new_usertype<lt::announce_endpoint>(
        "lt.announce_endpoint",
        sol::no_constructor,
        "enabled", sol::readonly(&lt::announce_endpoint::enabled),
        "info_hashes", sol::readonly(&lt::announce_endpoint::info_hashes));

    announce_endpoint_type["local_endpoint"] = [](const lt::announce_endpoint& ae)
    {
        return std::make_tuple(
            ae.local_endpoint.address().to_string(),
            ae.local_endpoint.port());
    };

    auto announce_entry_type = lua.new_usertype<lt::announce_entry>(
        "lt.announce_entry",
        sol::no_constructor,
        "endpoints",  sol::readonly(&lt::announce_entry::endpoints),
        "fail_limit", sol::readonly(&lt::announce_entry::fail_limit),
        "tier",       sol::readonly(&lt::announce_entry::tier),
        "trackerid",  sol::readonly(&lt::announce_entry::trackerid),
        "url",        sol::readonly(&lt::announce_entry::url));

    announce_entry_type["source"]   = sol::property([](const lt::announce_entry& ae) { return ae.source; });
    announce_entry_type["verified"] = sol::property([](const lt::announce_entry& ae) { return ae.verified; });

    auto file_storage_type = lua.new_usertype<lt::file_storage>(
        "lt.file_storage",
        sol::no_constructor,
        "file_name", [](const lt::file_storage& fs, int index) { return fs.file_name(lt::file_index_t{index}).to_string(); },
        "file_path", [](const lt::file_storage& fs, int index) { return fs.file_path(lt::file_index_t{index}); },
        "file_size", [](const lt::file_storage& fs, int index) { return fs.file_size(lt::file_index_t{index}); }
    );

    auto torrent_client_data_type = lua.new_usertype<TorrentClientData>(
        "porla.TorrentClientData",
        sol::no_constructor,
        "category", &TorrentClientData::category,
        "session",  sol::property([](const TorrentClientData& cd) -> std::optional<std::string>
                    {
                        if (auto s = cd.state.lock())
                        {
                            return s->name;
                        }

                        return std::nullopt;
                    }),
        "tags",     &TorrentClientData::tags);

    auto torrent_info_type = lua.new_usertype<lt::torrent_info>(
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
