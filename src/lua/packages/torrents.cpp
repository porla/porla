#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/file_storage.hpp>
#include <libtorrent/hex.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../plugin.hpp"
#include "../../config.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"
#include "../../utils/ratio.hpp"

using porla::Lua::Packages::Torrents;

static void ApplyPreset(lt::add_torrent_params& p, const porla::Config::Preset& preset)
{
    if (preset.download_limit.has_value())  p.download_limit  = preset.download_limit.value();
    if (preset.max_connections.has_value()) p.max_connections = preset.max_connections.value();
    if (preset.max_uploads.has_value())     p.max_uploads     = preset.max_uploads.value();
    if (preset.save_path.has_value())       p.save_path       = preset.save_path.value();
    if (preset.storage_mode.has_value())    p.storage_mode    = preset.storage_mode.value();
    if (preset.upload_limit.has_value())    p.upload_limit    = preset.upload_limit.value();

    // Set our custom client data
    if (preset.category.has_value())
        p.userdata.get<porla::TorrentClientData>()->category = preset.category.value();

    if (!preset.tags.empty())
        p.userdata.get<porla::TorrentClientData>()->tags = preset.tags;
}

static std::map<std::string, bool> FlagsToMap(const lt::torrent_flags_t& flags)
{
#define SET_INSERT_FLAG(name) \
    if ((flags & lt::torrent_flags:: name) == lt::torrent_flags:: name) \
    { \
        result_flags.insert({#name, true}); \
    } \
    else \
    { \
        result_flags.insert({#name, false}); \
    } \

    std::map<std::string, bool> result_flags;
    SET_INSERT_FLAG(seed_mode)
    SET_INSERT_FLAG(upload_mode)
    SET_INSERT_FLAG(share_mode)
    SET_INSERT_FLAG(apply_ip_filter)
    SET_INSERT_FLAG(paused)
    SET_INSERT_FLAG(auto_managed)
    SET_INSERT_FLAG(duplicate_is_error)
    SET_INSERT_FLAG(update_subscribe)
    SET_INSERT_FLAG(super_seeding)
    SET_INSERT_FLAG(sequential_download)
    SET_INSERT_FLAG(stop_when_ready)
    SET_INSERT_FLAG(override_trackers)
    SET_INSERT_FLAG(override_web_seeds)
    SET_INSERT_FLAG(need_save_resume)
    SET_INSERT_FLAG(disable_dht)
    SET_INSERT_FLAG(disable_lsd)
    SET_INSERT_FLAG(disable_pex)
    SET_INSERT_FLAG(no_verify_files)
    SET_INSERT_FLAG(default_dont_download)
    SET_INSERT_FLAG(i2p_torrent)
    return result_flags;
}

static void SetHandleFlags(const lt::torrent_handle& th, const sol::table& flags_tbl)
{
    lt::torrent_flags_t flags;
    lt::torrent_flags_t mask;

#define SET_FLAG(name) \
    if (flags_tbl[#name].valid()) \
    { \
        mask |= lt::torrent_flags:: name; \
        if (flags_tbl[#name].get<bool>()) \
        { \
            flags |= lt::torrent_flags:: name; \
        } \
    }

    SET_FLAG(seed_mode)
    SET_FLAG(upload_mode)
    SET_FLAG(share_mode)
    SET_FLAG(apply_ip_filter)
    SET_FLAG(paused)
    SET_FLAG(auto_managed)
    SET_FLAG(duplicate_is_error)
    SET_FLAG(update_subscribe)
    SET_FLAG(super_seeding)
    SET_FLAG(sequential_download)
    SET_FLAG(stop_when_ready)
    SET_FLAG(override_trackers)
    SET_FLAG(override_web_seeds)
    SET_FLAG(need_save_resume)
    SET_FLAG(disable_dht)
    SET_FLAG(disable_lsd)
    SET_FLAG(disable_pex)
    SET_FLAG(no_verify_files)
    SET_FLAG(default_dont_download)
    SET_FLAG(i2p_torrent)

    th.set_flags(flags, mask);
}

void Torrents::Register(sol::state& lua)
{
    auto announce_infohash_type = lua.new_usertype<lt::announce_infohash>(
        "lt.AnnounceInfoHash",
        sol::no_constructor);

    announce_infohash_type["complete_sent"] = sol::property([](const lt::announce_infohash& aih) { return aih.complete_sent; });
    announce_infohash_type["fails"] = sol::property([](const lt::announce_infohash& aih) { return aih.fails; });
    announce_infohash_type["last_error"] = sol::property([](const lt::announce_infohash& aih) { return aih.last_error; });
    announce_infohash_type["message"] = sol::property([](const lt::announce_infohash& aih) { return aih.message; });
    announce_infohash_type["min_announce"] = sol::property([](const lt::announce_infohash& aih) { return aih.min_announce.time_since_epoch().count(); });
    announce_infohash_type["next_announce"] = sol::property([](const lt::announce_infohash& aih) { return aih.next_announce.time_since_epoch().count(); });
    announce_infohash_type["scrape_complete"] = sol::property([](const lt::announce_infohash& aih) { return aih.scrape_complete; });
    announce_infohash_type["scrape_downloaded"] = sol::property([](const lt::announce_infohash& aih) { return aih.scrape_downloaded; });
    announce_infohash_type["scrape_incomplete"] = sol::property([](const lt::announce_infohash& aih) { return aih.scrape_incomplete; });
    announce_infohash_type["start_sent"] = sol::property([](const lt::announce_infohash& aih) { return aih.start_sent; });
    announce_infohash_type["updating"] = sol::property([](const lt::announce_infohash& aih) { return aih.updating; });

    auto announce_endpoint_type = lua.new_usertype<lt::announce_endpoint>(
        "lt.AnnounceEndpoint",
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
        "lt.AnnounceEntry",
        sol::no_constructor,
        "endpoints",  sol::readonly(&lt::announce_entry::endpoints),
        "fail_limit", sol::readonly(&lt::announce_entry::fail_limit),
        "tier",       sol::readonly(&lt::announce_entry::tier),
        "trackerid",  sol::readonly(&lt::announce_entry::trackerid),
        "url",        sol::readonly(&lt::announce_entry::url));

    announce_entry_type["source"]   = sol::property([](const lt::announce_entry& ae) { return ae.source; });
    announce_entry_type["verified"] = sol::property([](const lt::announce_entry& ae) { return ae.verified; });

    auto file_storage_type = lua.new_usertype<lt::file_storage>(
        "FileStorage",
        sol::no_constructor,
        "file_name", [](const lt::file_storage& fs, int index) { return fs.file_name(lt::file_index_t{index}).to_string(); },
        "file_path", [](const lt::file_storage& fs, int index) { return fs.file_path(lt::file_index_t{index}); },
        "file_size", [](const lt::file_storage& fs, int index) { return fs.file_size(lt::file_index_t{index}); }
        );

    auto info_hash_type = lua.new_usertype<lt::info_hash_t>(
        "lt.InfoHash",
        sol::no_constructor);

    info_hash_type["v1"] = sol::property(
        [](const lt::info_hash_t& ih) -> std::optional<std::string>
        {
            return ih.has_v1() ? std::optional(lt::aux::to_hex(ih.v1)) : std::nullopt;
        });

    info_hash_type["v2"] = sol::property(
        [](const lt::info_hash_t& ih) -> std::optional<std::string>
        {
            return ih.has_v2() ? std::optional(lt::aux::to_hex(ih.v2)) : std::nullopt;
        });

    auto peer_info_type = lua.new_usertype<lt::peer_info>(
        "lt.PeerInfo",
        sol::no_constructor,
        "busy_requests",         sol::readonly(&lt::peer_info::busy_requests),
        "client",                sol::readonly(&lt::peer_info::client),
        "connection_type",       sol::readonly(&lt::peer_info::connection_type),
        "down_speed",            sol::readonly(&lt::peer_info::down_speed),
        "download_queue_length", sol::readonly(&lt::peer_info::download_queue_length),
        "download_queue_time",   sol::readonly(&lt::peer_info::download_queue_time),
        "flags",                 sol::readonly(&lt::peer_info::flags),
        "progress",              sol::readonly(&lt::peer_info::progress),
        "rtt",                   sol::readonly(&lt::peer_info::rtt),
        "source",                sol::readonly(&lt::peer_info::source),
        "total_download",        sol::readonly(&lt::peer_info::total_download),
        "total_upload",          sol::readonly(&lt::peer_info::total_upload),
        "up_speed",              sol::readonly(&lt::peer_info::up_speed));

    peer_info_type["last_active"]  = sol::property([](const lt::peer_info& pi) { return pi.last_active.count(); });
    peer_info_type["last_request"] = sol::property([](const lt::peer_info& pi) { return pi.last_request.count(); });

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
        "TorrentInfo",
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

    auto torrent_handle_type = lua.new_usertype<lt::torrent_handle>(
        "lt.TorrentHandle",
        sol::no_constructor,
        "clear_error",             &lt::torrent_handle::clear_error,
        "clear_peers",             &lt::torrent_handle::clear_peers,
        "download_limit",          &lt::torrent_handle::download_limit,
        "file_priorities",         &lt::torrent_handle::get_file_priorities,
        "flags",                   [](const lt::torrent_handle& th)
                                   {
                                       return FlagsToMap(th.flags());
                                   },
        "flush_cache",             &lt::torrent_handle::flush_cache,
        "force_reannounce",        [](const lt::torrent_handle& th, const sol::table& args)
                                   {
                                       int seconds = 0;
                                       int index   = -1;

                                       if (args["seconds"].is<int>())       seconds = args["seconds"];
                                       if (args["tracker_index"].is<int>()) index   = args["tracker_index"];

                                       th.force_reannounce(seconds, index);
                                   },
        "force_recheck",           &lt::torrent_handle::force_recheck,
        "info_hash",               &lt::torrent_handle::info_hashes,
        "is_valid",                &lt::torrent_handle::is_valid,
        "max_connections",         &lt::torrent_handle::max_connections,
        "max_uploads",             &lt::torrent_handle::max_uploads,
        "move_storage",            [](const lt::torrent_handle& th, const std::string& path) { th.move_storage(path); },
        "pause",                   [](const lt::torrent_handle& th) { return th.pause(); },
        "peer_info",               [](const lt::torrent_handle& th)
                                   {
                                       std::vector<lt::peer_info> peers;
                                       th.get_peer_info(peers);
                                       return peers;
                                   },
        "post_download_queue",     &lt::torrent_handle::post_download_queue,
        "post_file_progress",      [](const lt::torrent_handle& th) { return th.post_file_progress(lt::torrent_handle::piece_granularity); },
        "post_peer_info",          &lt::torrent_handle::post_peer_info,
        "post_piece_availability", &lt::torrent_handle::post_piece_availability,
        "post_status",             [](const lt::torrent_handle& th) { return th.post_status(); },
        "post_trackers",           &lt::torrent_handle::post_trackers,
        "prioritize_files",        [](const lt::torrent_handle& th, const sol::table& args)
        {
            printf("hhh");
            std::vector<int> priorities = args.as<std::vector<int>>();
            std::vector<lt::download_priority_t> prios;
            std::for_each(
                priorities.begin(),
                priorities.end(),
                [&prios](int i) { prios.emplace_back(i); });
printf("Hejej");
            th.prioritize_files(prios);
        },
        "queue_position",          &lt::torrent_handle::queue_position,
        "queue_position_bottom",   &lt::torrent_handle::queue_position_bottom,
        "queue_position_down",     &lt::torrent_handle::queue_position_down,
        "queue_position_top",      &lt::torrent_handle::queue_position_top,
        "queue_position_up",       &lt::torrent_handle::queue_position_up,
        "resume",                  &lt::torrent_handle::resume,
        "set_download_limit",      &lt::torrent_handle::set_download_limit,
        "set_flags",               [](const lt::torrent_handle& th, const sol::table& flags) { SetHandleFlags(th, flags); },
        "set_max_connections",     &lt::torrent_handle::set_max_connections,
        "set_max_uploads",         &lt::torrent_handle::set_max_uploads,
        "set_upload_limit",        &lt::torrent_handle::set_upload_limit,
        "status",                  [](const lt::torrent_handle& th) { return th.status(); },
        "trackers",                &lt::torrent_handle::trackers,
        "torrent_file",            &lt::torrent_handle::torrent_file,
        "upload_limit",            &lt::torrent_handle::upload_limit,
        "userdata",                [](const lt::torrent_handle& th) { return th.userdata().get<TorrentClientData>(); });

    auto torrent_status_type = lua.new_usertype<lt::torrent_status>(
        "lt.TorrentStatus",
        sol::no_constructor,
        "active_duration",        sol::property([](const lt::torrent_status& ts) { return ts.active_duration.count(); }),
        "added_time",             sol::readonly(&lt::torrent_status::added_time),
        "all_time_download",      sol::readonly(&lt::torrent_status::all_time_download),
        "all_time_upload",        sol::readonly(&lt::torrent_status::all_time_upload),
        "announcing_to_dht",      sol::readonly(&lt::torrent_status::announcing_to_dht),
        "announcing_to_lsd",      sol::readonly(&lt::torrent_status::announcing_to_lsd),
        "announcing_to_trackers", sol::readonly(&lt::torrent_status::announcing_to_trackers),
        "block_size",             sol::readonly(&lt::torrent_status::block_size),
        "completed_time",         sol::readonly(&lt::torrent_status::completed_time),
        "connect_candidates",     sol::readonly(&lt::torrent_status::connect_candidates),
        "connections_limit",      sol::readonly(&lt::torrent_status::connections_limit),
        "current_tracker",        sol::readonly(&lt::torrent_status::current_tracker),
        "distributed_copies",     sol::readonly(&lt::torrent_status::distributed_copies),
        "down_bandwidth_queue",   sol::readonly(&lt::torrent_status::down_bandwidth_queue),
        "download_payload_rate",  sol::readonly(&lt::torrent_status::download_payload_rate),
        "download_rate",          sol::readonly(&lt::torrent_status::download_rate),
        // errc
        // error_file
        "finished_duration",      sol::property([](const lt::torrent_status& ts) { return ts.finished_duration.count(); }),
        "flags",                  sol::property([](const lt::torrent_status& ts) { return FlagsToMap(ts.flags); }),
        "info_hash",              sol::readonly(&lt::torrent_status::info_hashes),
        "is_finished",            sol::readonly(&lt::torrent_status::is_finished),
        "is_seeding",             sol::readonly(&lt::torrent_status::is_seeding),
        "handle",                 sol::readonly(&lt::torrent_status::handle),
        "has_incoming",           sol::readonly(&lt::torrent_status::has_incoming),
        "has_metadata",           sol::readonly(&lt::torrent_status::has_metadata),
        // last download
        // last seen complete
        // last upload
        "list_peers",             sol::readonly(&lt::torrent_status::list_peers),
        "list_seeds",             sol::readonly(&lt::torrent_status::list_seeds),
        "moving_storage",         sol::readonly(&lt::torrent_status::moving_storage),
        "name",                   sol::readonly(&lt::torrent_status::name),
        "need_save_resume",       sol::readonly(&lt::torrent_status::need_save_resume),
        // next announce
        "num_complete",           sol::readonly(&lt::torrent_status::num_complete),
        "num_connections",        sol::readonly(&lt::torrent_status::num_connections),
        "num_incomplete",         sol::readonly(&lt::torrent_status::num_incomplete),
        "num_peers",              sol::readonly(&lt::torrent_status::num_peers),
        "num_pieces",             sol::readonly(&lt::torrent_status::num_pieces),
        "num_seeds",              sol::readonly(&lt::torrent_status::num_seeds),
        "num_uploads",            sol::readonly(&lt::torrent_status::num_uploads),
        // pieces
        "progress",               sol::readonly(&lt::torrent_status::progress),
        "queue_position",         sol::readonly(&lt::torrent_status::queue_position),
        "ratio",                  sol::property([](const lt::torrent_status& ts) { return porla::Utils::Ratio(ts); }),
        "save_path",              sol::readonly(&lt::torrent_status::save_path),
        "seed_rank",              sol::readonly(&lt::torrent_status::seed_rank),
        "seeding_duration",       sol::property([](const lt::torrent_status& ts) { return ts.seeding_duration.count(); }),
        "state",                  sol::property([](const lt::torrent_status& ts) -> std::optional<std::string>
                                  {
                                      switch (ts.state)
                                      {
                                          case lt::torrent_status::checking_files:
                                              return "checking_files";
                                          case lt::torrent_status::downloading_metadata:
                                              return "downloading_metadata";
                                          case lt::torrent_status::downloading:
                                              return "downloading";
                                          case lt::torrent_status::finished:
                                              return "finished";
                                          case lt::torrent_status::seeding:
                                              return "seeding";
                                          case lt::torrent_status::checking_resume_data:
                                              return "checking_resume_data";
                                      }

                                      return std::nullopt;
                                  }),
        // storage mode
        "torrent_file",           sol::property([](const lt::torrent_status& ts) -> std::shared_ptr<const lt::torrent_info>
                                  {
                                      if (auto tf = ts.torrent_file.lock()) return tf;
                                      return nullptr;
                                  }),
        "total",                  sol::readonly(&lt::torrent_status::total),
        "total_done",             sol::readonly(&lt::torrent_status::total_done),
        "total_download",         sol::readonly(&lt::torrent_status::total_download),
        "total_failed_bytes",     sol::readonly(&lt::torrent_status::total_failed_bytes),
        "total_payload_download", sol::readonly(&lt::torrent_status::total_payload_download),
        "total_payload_upload",   sol::readonly(&lt::torrent_status::total_payload_upload),
        "total_redundant_bytes",  sol::readonly(&lt::torrent_status::total_redundant_bytes),
        "total_upload",           sol::readonly(&lt::torrent_status::total_upload),
        "up_bandwidth_queue",     sol::readonly(&lt::torrent_status::up_bandwidth_queue),
        "upload_payload_rate",    sol::readonly(&lt::torrent_status::upload_payload_rate),
        "upload_rate",            sol::readonly(&lt::torrent_status::upload_rate),
        "uploads_limit",          sol::readonly(&lt::torrent_status::uploads_limit)
        // verified pieces
        );

    lua["package"]["preload"]["torrents"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table torrents = lua.create_table();

        return torrents;
    };
}
