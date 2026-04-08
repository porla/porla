#include "../types.hpp"

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../../torrentclientdata.hpp"

using porla::TorrentClientData;
using porla::Lua::Types::LtAddTorrentParams;

struct FilePrioritiesProxy
{
    lt::add_torrent_params& params;

    lt::download_priority_t get(int index)
    {
        return params.file_priorities.at(index - 1);
    }

    void set(int index, lt::download_priority_t prio)
    {
        int i = index - 1;

        if (i >= static_cast<int>(params.file_priorities.size()))
        {
            params.file_priorities.resize(i + 1, lt::default_priority);
        }

        params.file_priorities[i] = prio;
    }

    void append(lt::download_priority_t prio)
    {
        params.file_priorities.push_back(prio);
    }

    int size()
    {
        return static_cast<int>(params.file_priorities.size());
    }
};

void LtAddTorrentParams::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["add_torrent_params"] = lua.new_usertype<lt::add_torrent_params>(
        "lt.add_torrent_params",
        sol::factories([]()
        {
            auto atp = std::make_shared<lt::add_torrent_params>();
            atp->userdata = lt::client_data_t(new TorrentClientData());
            return atp;
        }),
        "from_magnet",     sol::factories([](const std::string& uri) -> std::pair<std::shared_ptr<lt::add_torrent_params>, std::optional<std::string>>
                           {
                               auto atp = std::make_shared<lt::add_torrent_params>();
                               atp->userdata = lt::client_data_t(new TorrentClientData());

                               lt::error_code ec;
                               lt::parse_magnet_uri(uri, *atp, ec);

                               if (ec)
                               {
                                   return std::pair(nullptr, ec.message());
                               }

                               return std::pair(atp, std::nullopt);
                           }),

        "download_limit",  &lt::add_torrent_params::download_limit,
        "file_priorities", sol::property([](lt::add_torrent_params& p) { return FilePrioritiesProxy{p}; }),
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
        "userdata",        sol::property([](const lt::add_torrent_params& p) { return p.userdata.get<TorrentClientData>(); }));

    lua.new_usertype<FilePrioritiesProxy>(
        "porla.internal.FilePrioritiesProxy",
        sol::no_constructor,
        sol::meta_function::index,     &FilePrioritiesProxy::get,
        sol::meta_function::new_index, &FilePrioritiesProxy::set,
        sol::meta_function::length,    &FilePrioritiesProxy::size,
        "append",                      &FilePrioritiesProxy::append);
}