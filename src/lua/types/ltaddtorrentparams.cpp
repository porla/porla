#include "ltaddtorrentparams.hpp"

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/load_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "poerror.hpp"
#include "../../torrentclientdata.hpp"

using porla::TorrentClientData;
using porla::Lua::Types::LtAddTorrentParams;

void LtAddTorrentParams::Register(sol::state& lua)
{
    sol::table atp = lua.create_table();

    atp.set_function("from_path", [](sol::this_state ts, const std::string& path) -> std::tuple<sol::object, sol::object>
    {
        lt::error_code ec;
        lt::add_torrent_params params = lt::load_torrent_file(path, ec, {});

        if (ec)
        {
            return std::make_tuple(sol::nil, PoError::Construct(ts, ec));
        }

        return std::make_tuple(ToTable(ts, params), sol::nil);
    });

    lua["LtAddTorrentParams"] = atp;
}

lt::add_torrent_params LtAddTorrentParams::ToParams(const sol::object& params)
{
    const sol::table t = params.as<sol::table>();

    lt::add_torrent_params atp;

    if (sol::optional<sol::table> mt = t[sol::metatable_key])
    {
        if (sol::optional<std::shared_ptr<lt::add_torrent_params>> base = (*mt)["__atp"])
        {
            if (*base) atp = **base;
        }
    }

    if (sol::optional<lt::info_hash_t> v = t["info_hash"]) atp.info_hashes = *v;
    if (sol::optional<std::string>     v = t["save_path"]) atp.save_path   = *v;

    return atp;
}

sol::table LtAddTorrentParams::ToTable(sol::this_state ts, const lt::add_torrent_params& params)
{
    sol::state_view lua(ts);

    sol::table file_priorities = lua.create_table();
    for (const auto prio : params.file_priorities) { file_priorities.add(static_cast<std::uint8_t>(prio)); }

    sol::table tracker_tiers = lua.create_table();
    for (const auto tier : params.tracker_tiers) { tracker_tiers.add(tier); }

    sol::table trackers = lua.create_table();
    for (const auto& tracker : params.trackers) { trackers.add(tracker); }

    sol::table url_seeds = lua.create_table();
    for (const auto& url : params.url_seeds) { url_seeds.add(url); }

    sol::table meta = lua.create_table();
    meta["__atp"]   = std::make_shared<lt::add_torrent_params>(params);
    meta["__name"]  = "lt::add_torrent_params";

    sol::table tbl = lua.create_table();
    tbl[sol::metatable_key]   = meta;

    tbl["active_time"]        = params.active_time;
    tbl["added_time"]         = params.added_time;
    // banned peers
    tbl["comment"]            = params.comment;
    tbl["completed_time"]     = params.completed_time;
    tbl["created_by"]         = params.created_by;
    tbl["creation_date"]      = params.creation_date;
    // dht nodes
    tbl["download_limit"]     = params.download_limit;
    tbl["file_priorities"]    = file_priorities;
    tbl["finished_time"]      = params.finished_time;
    tbl["flags"]              = params.flags;
    tbl["info_hash"]          = params.info_hashes;
    tbl["last_download"]      = params.last_download;
    tbl["last_seen_complete"] = params.last_seen_complete;
    tbl["last_upload"]        = params.last_upload;
    tbl["max_connections"]    = params.max_connections;
    tbl["max_uploads"]        = params.max_uploads;
    tbl["name"]               = params.name;
    tbl["num_complete"]       = params.num_complete;
    tbl["num_downloaded"]     = params.num_downloaded;
    tbl["num_incomplete"]     = params.num_incomplete;
    tbl["part_file_dir"]      = params.part_file_dir;
    tbl["root_certificate"]   = params.root_certificate;
    // peers
    tbl["save_path"]          = params.save_path;
    tbl["seeding_time"]       = params.seeding_time;
    tbl["storage_mode"]       = params.storage_mode == lt::storage_mode_allocate ? "allocate" : "sparse";
    tbl["ti"]                 = params.ti;
    tbl["trackerid"]          = params.trackerid;
    tbl["tracker_tiers"]      = tracker_tiers;
    tbl["trackers"]           = trackers;
    tbl["total_downloaded"]   = params.total_downloaded;
    tbl["total_uploaded"]     = params.total_uploaded;
    tbl["upload_limit"]       = params.upload_limit;
    tbl["url_seeds"]          = url_seeds;
    // userdata

    return tbl;
}