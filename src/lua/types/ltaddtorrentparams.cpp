#include "../types.hpp"

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../../torrentclientdata.hpp"

using porla::TorrentClientData;
using porla::Lua::Types::LtAddTorrentParams;

void LtAddTorrentParams::Register(sol::state& lua)
{
    lua.new_usertype<lt::add_torrent_params>(
        "LtAddTorrentParams",
        sol::call_constructor, sol::factories([]()
        {
            lt::add_torrent_params atp;
            atp.userdata = lt::client_data_t(new TorrentClientData());
            return atp;
        }),
        "from_magnet",     [](const std::string& uri) -> std::pair<std::optional<lt::add_torrent_params>, std::optional<std::string>>
                           {
                               lt::add_torrent_params atp;
                               lt::error_code ec;

                               lt::parse_magnet_uri(uri, atp, ec);

                               if (ec)
                               {
                                   return std::pair(std::nullopt, ec.message());
                               }

                               atp.userdata = lt::client_data_t(new TorrentClientData());

                               return std::pair(atp, std::nullopt);
                           },

        "download_limit",  &lt::add_torrent_params::download_limit,
        // "file_priorities", sol::property([](lt::add_torrent_params& p) { return FilePrioritiesProxy{p}; }),
        "flags",           &lt::add_torrent_params::flags,
        "info_hash",       &lt::add_torrent_params::info_hashes,
        "max_connections", &lt::add_torrent_params::max_connections,
        "max_uploads",     &lt::add_torrent_params::max_uploads,
        "name",            &lt::add_torrent_params::name,
        "save_path",       &lt::add_torrent_params::save_path,
        "storage_mode",    sol::property(
            [](const lt::add_torrent_params& p) -> lt::storage_mode_t { return p.storage_mode; },
            [](lt::add_torrent_params& p, lt::storage_mode_t m) { p.storage_mode = m; }
        ),
        "ti",              &lt::add_torrent_params::ti,
        "tracker_tiers",   &lt::add_torrent_params::tracker_tiers,
        "trackerid",       &lt::add_torrent_params::trackerid,
        "trackers",        &lt::add_torrent_params::trackers,
        "upload_limit",    &lt::add_torrent_params::upload_limit,
        "userdata",        sol::property([](const lt::add_torrent_params& p) { return p.userdata.get<TorrentClientData>(); })
    );
}