#include "torrents.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/hex.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../plugins/plugin.hpp"
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
        "tags",     &TorrentClientData::tags);

    auto torrent_info_type = lua.new_usertype<lt::torrent_info>(
        "lt.TorrentInfo",
        sol::no_constructor,
        "comment",    &lt::torrent_info::comment,
        "creator",    &lt::torrent_info::creator,
        "info_hash",  &lt::torrent_info::info_hashes,
        "name",       &lt::torrent_info::name,
        "num_files",  &lt::torrent_info::num_files,
        "num_pieces", &lt::torrent_info::num_pieces,
        "priv",       &lt::torrent_info::priv,
        "trackers",   &lt::torrent_info::trackers,
        "total_size", &lt::torrent_info::total_size);

    auto torrent_handle_type = lua.new_usertype<lt::torrent_handle>(
        "lt.TorrentHandle",
        sol::no_constructor,
        "clear_error",             &lt::torrent_handle::clear_error,
        "clear_peers",             &lt::torrent_handle::clear_peers,
        "download_limit",          &lt::torrent_handle::download_limit,
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
        "post_peer_info",          &lt::torrent_handle::post_peer_info,
        "post_piece_availability", &lt::torrent_handle::post_piece_availability,
        "post_status",             [](const lt::torrent_handle& th) { return th.post_status(); },
        "post_trackers",           &lt::torrent_handle::post_trackers,
        "queue_position",          &lt::torrent_handle::queue_position,
        "queue_position_bottom",   &lt::torrent_handle::queue_position_bottom,
        "queue_position_down",     &lt::torrent_handle::queue_position_down,
        "queue_position_top",      &lt::torrent_handle::queue_position_top,
        "queue_position_up",       &lt::torrent_handle::queue_position_up,
        "resume",                  &lt::torrent_handle::resume,
        "set_download_limit",      &lt::torrent_handle::set_download_limit,
        "set_max_connections",     &lt::torrent_handle::set_max_connections,
        "set_max_uploads",         &lt::torrent_handle::set_max_uploads,
        "set_upload_limit",        &lt::torrent_handle::set_upload_limit,
        "status",                  [](const lt::torrent_handle& th) { return th.status(); },
        "trackers",                &lt::torrent_handle::trackers,
        "upload_limit",            &lt::torrent_handle::upload_limit,
        "userdata",                [](const lt::torrent_handle& th) { return th.userdata().get<TorrentClientData>(); });

    auto torrent_status_type = lua.new_usertype<lt::torrent_status>(
        "lt.TorrentStatus",
        sol::no_constructor,
        // active duration
        // added time
        "all_time_download",      sol::readonly(&lt::torrent_status::all_time_download),
        "all_time_upload",        sol::readonly(&lt::torrent_status::all_time_upload),
        "announcing_to_dht",      sol::readonly(&lt::torrent_status::announcing_to_dht),
        "announcing_to_lsd",      sol::readonly(&lt::torrent_status::announcing_to_lsd),
        "announcing_to_trackers", sol::readonly(&lt::torrent_status::announcing_to_trackers),
        "block_size",             sol::readonly(&lt::torrent_status::block_size),
        // completed time
        "connect_candidates",     sol::readonly(&lt::torrent_status::connect_candidates),
        "connections_limit",      sol::readonly(&lt::torrent_status::connections_limit),
        "current_tracker",        sol::readonly(&lt::torrent_status::current_tracker),
        "distributed_copies",     sol::readonly(&lt::torrent_status::distributed_copies),
        "down_bandwidth_queue",   sol::readonly(&lt::torrent_status::down_bandwidth_queue),
        "download_payload_rate",  sol::readonly(&lt::torrent_status::download_payload_rate),
        "download_rate",          sol::readonly(&lt::torrent_status::download_rate),
        // errc
        // error_file
        // finished duration
        // flags
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
        "save_path",              sol::readonly(&lt::torrent_status::save_path),
        "seed_rank",              sol::readonly(&lt::torrent_status::seed_rank),
        // seeding duration
        // state
        // storage mode
        "torrent_file",           sol::readonly(&lt::torrent_status::torrent_file),
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

    torrent_status_type["category"] = sol::property(
        [](const lt::torrent_status& ts)
        {
            const TorrentClientData* client_data = ts.handle.userdata().get<TorrentClientData>();
            return client_data->category;
        });

    torrent_status_type["flags"] = sol::property(
        [](const lt::torrent_status& ts)
        {
#define SET_INSERT_FLAG(name) if ((ts.flags & lt::torrent_flags:: name) == lt::torrent_flags:: name) flags.insert(#name);

            std::set<std::string> flags;
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

            return flags;
        });

    lua["package"]["preload"]["torrents"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table torrents = lua.create_table();

        torrents["add"] = [](sol::this_state s, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            const auto& state = args["preset"].is<std::string>()
                ? options.config.presets.find(args["preset"]) != options.config.presets.end()
                  ? options.config.presets.at(args["preset"]).session.has_value()
                    ? options.sessions.Get(options.config.presets.at(args["preset"]).session.value())
                    : options.sessions.Default()
                  : options.sessions.Default()
                : options.config.presets.find("default") != options.config.presets.end()
                  ? options.config.presets.at("default").session.has_value()
                    ? options.sessions.Get(options.config.presets.at("default").session.value())
                    : options.sessions.Default()
                  : options.sessions.Default();

            lt::add_torrent_params p;
            p.userdata = lt::client_data_t(new TorrentClientData());
            p.userdata.get<TorrentClientData>()->state = state;

            // Apply the 'default' preset if it exists
            if (options.config.presets.find("default") != options.config.presets.end())
            {
                ApplyPreset(p, options.config.presets.at("default"));
            }

            if (args["preset"].is<std::string>())
            {
                const auto preset_name = args["preset"].get<std::string>();
                const auto preset = options.config.presets.find(preset_name);

                if (preset == options.config.presets.end())
                {
                    BOOST_LOG_TRIVIAL(warning) << "Specified preset '" << preset_name << "' not found.";
                }
                // Only apply presets other than default here, since default is applied above..
                else if (preset_name != "default")
                {
                    BOOST_LOG_TRIVIAL(debug) << "Applying preset " << preset_name;
                    ApplyPreset(p, preset->second);
                }
            }

            if (args["ti"].is<std::shared_ptr<lt::torrent_info>>())
            {
                p.ti = args["ti"].get<std::shared_ptr<lt::torrent_info>>();

                for (const auto& [ name, s ]: options.sessions.All())
                {
                    if (s->torrents.find(p.ti->info_hashes()) != s->torrents.end())
                    {
                        BOOST_LOG_TRIVIAL(error) << "Torrent already in session";
                        return;
                    }
                }
            }
            else if (args["magnet_uri"].is<std::string>())
            {
                lt::error_code ec;
                lt::parse_magnet_uri(args["magnet_uri"].get<std::string>(), p, ec);

                if (ec)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to parse magnet uri: " << ec.message();
                    return;
                }
            }

            if (args["download_limit"].valid())  p.download_limit  = args["download_limit"].get<int>();
            if (args["http_seeds"].valid())      p.http_seeds      = args["http_seeds"].get<std::vector<std::string>>();
            if (args["max_connections"].valid()) p.max_connections = args["max_connections"].get<int>();
            if (args["max_uploads"].valid())     p.max_uploads     = args["max_uploads"].get<int>();
            if (args["name"].valid())            p.name            = args["name"].get<std::string>();
            if (args["save_path"].valid())       p.save_path       = args["save_path"].get<std::string>();
            if (args["trackers"].valid())        p.trackers        = args["trackers"].get<std::vector<std::string>>();
            if (args["upload_limit"].valid())    p.upload_limit    = args["upload_limit"].get<int>();
            if (args["url_seeds"].valid())       p.url_seeds       = args["url_seeds"].get<std::vector<std::string>>();

            // userdata values
            if (args["category"].valid())        p.userdata.get<TorrentClientData>()->category = args["category"].get<std::string>();
            if (args["tags"].valid())            p.userdata.get<TorrentClientData>()->tags     = args["tags"].get<std::unordered_set<std::string>>();

            state->session->async_add_torrent(p);
        };

        torrents["errors"] = lua.create_table();
        torrents["errors"]["tracker_failure"] = lt::errors::tracker_failure;

        torrents["parse"] = [](const std::string& data, const std::string& type)
        {
            if (type == "buffer")
            {
                return std::make_shared<lt::torrent_info>(data, lt::from_span);
            }

            if (type == "magnet")
            {
                // return lt::parse_magnet_uri(data);
            }

            return std::make_shared<lt::torrent_info>(data);
        };

        torrents["properties"] = lua.create_table();
        torrents["properties"]["get"] = [](sol::this_state s, const lt::torrent_status& ts) -> sol::reference
        {
            if (!ts.handle.is_valid())
            {
                return sol::nil;
            }

            sol::state_view lua{s};
            sol::table p = lua.create_table();

#define INSERT_FLAG(name) flags.insert({ #name, (ts.flags & lt::torrent_flags:: name) == lt::torrent_flags:: name });

            std::map<std::string, bool> flags;
            INSERT_FLAG(seed_mode)
            INSERT_FLAG(upload_mode)
            INSERT_FLAG(share_mode)
            INSERT_FLAG(apply_ip_filter)
            INSERT_FLAG(paused)
            INSERT_FLAG(auto_managed)
            INSERT_FLAG(duplicate_is_error)
            INSERT_FLAG(update_subscribe)
            INSERT_FLAG(super_seeding)
            INSERT_FLAG(sequential_download)
            INSERT_FLAG(stop_when_ready)
            INSERT_FLAG(override_trackers)
            INSERT_FLAG(override_web_seeds)
            INSERT_FLAG(need_save_resume)
            INSERT_FLAG(disable_dht)
            INSERT_FLAG(disable_lsd)
            INSERT_FLAG(disable_pex)
            INSERT_FLAG(no_verify_files)
            INSERT_FLAG(default_dont_download)
            INSERT_FLAG(i2p_torrent)

            p["flags"]           = flags;

            return p;
        };

        torrents["properties"]["set"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            if (!ts.handle.is_valid())
            {
                return;
            }

            lt::torrent_flags_t flags;
            lt::torrent_flags_t mask;

            if (args["flags"].valid())
            {
                const auto& flags_tbl = args["flags"].get<sol::table>();

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

                ts.handle.set_flags(flags, mask);
            }
        };

        return torrents;
    };
}
