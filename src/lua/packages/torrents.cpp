#include "torrents.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>

#include "../plugins/plugin.hpp"
#include "../../config.hpp"
#include "../../session.hpp"
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
        sol::no_constructor);

    announce_endpoint_type["enabled"] = sol::property([](const lt::announce_endpoint& ae) { return ae.enabled; });
    announce_endpoint_type["info_hashes"] = sol::property([](const lt::announce_endpoint& ae) { return ae.info_hashes; });
    announce_endpoint_type["local_endpoint"] = [](const lt::announce_endpoint& ae)
    {
        return std::make_tuple(
            ae.local_endpoint.address().to_string(),
            ae.local_endpoint.port());
    };

    auto announce_entry_type = lua.new_usertype<lt::announce_entry>(
        "lt.AnnounceEntry",
        sol::no_constructor);

    announce_entry_type["endpoints"] = sol::property([](const lt::announce_entry& ae) { return ae.endpoints; });
    announce_entry_type["fail_limit"] = sol::property([](const lt::announce_entry& ae) { return ae.fail_limit; });
    announce_entry_type["source"] = sol::property([](const lt::announce_entry& ae) { return ae.source; });
    announce_entry_type["tier"] = sol::property([](const lt::announce_entry& ae) { return ae.tier; });
    announce_entry_type["trackerid"] = sol::property([](const lt::announce_entry& ae) { return ae.trackerid; });
    announce_entry_type["url"] = sol::property([](const lt::announce_entry& ae) { return ae.url; });
    announce_entry_type["verified"] = sol::property([](const lt::announce_entry& ae) { return ae.verified; });

    auto peer_info_type = lua.new_usertype<lt::peer_info>(
        "lt.PeerInfo",
        sol::no_constructor);

    peer_info_type["busy_requests"] = sol::property([](const lt::peer_info& pi) { return pi.busy_requests; });
    peer_info_type["client"] = sol::property([](const lt::peer_info& pi) { return pi.client; });
    peer_info_type["connection_type"] = sol::property([](const lt::peer_info& pi) { return pi.connection_type; });
    peer_info_type["down_speed"] = sol::property([](const lt::peer_info& pi) { return pi.down_speed; });
    peer_info_type["up_speed"] = sol::property([](const lt::peer_info& pi) { return pi.up_speed; });
    peer_info_type["download_queue_length"] = sol::property([](const lt::peer_info& pi) { return pi.download_queue_length; });
    peer_info_type["download_queue_time"] = sol::property([](const lt::peer_info& pi) { return pi.download_queue_time.count(); });
    peer_info_type["flags"] = sol::property([](const lt::peer_info& pi) { return pi.flags; });
    // peer_info_type["flags"] = sol::property([](const lt::peer_info& pi) { return pi.flags; });
    peer_info_type["last_active"] = sol::property([](const lt::peer_info& pi) { return pi.last_active.count(); });
    peer_info_type["last_request"] = sol::property([](const lt::peer_info& pi) { return pi.last_request.count(); });
    // local endpoint peer_info_type["last_request"] = sol::property([](const lt::peer_info& pi) { return pi.last_request.count(); });
    peer_info_type["progress"] = sol::property([](const lt::peer_info& pi) { return pi.progress; });
    peer_info_type["rtt"] = sol::property([](const lt::peer_info& pi) { return pi.rtt; });
    peer_info_type["source"] = sol::property([](const lt::peer_info& pi) { return pi.source; });
    peer_info_type["total_download"] = sol::property([](const lt::peer_info& pi) { return pi.total_download; });
    peer_info_type["total_upload"] = sol::property([](const lt::peer_info& pi) { return pi.total_upload; });

    auto torrent_status_type = lua.new_usertype<lt::torrent_status>(
        "lt.TorrentStatus",
        sol::no_constructor);

    torrent_status_type["category"] = sol::property(
        [](const lt::torrent_status& ts)
        {
            const TorrentClientData* client_data = ts.handle.userdata().get<TorrentClientData>();
            return client_data->category;
        });

    torrent_status_type["active_duration"]   = sol::property([](const lt::torrent_status& ts) { return ts.active_duration.count(); });
    torrent_status_type["current_tracker"]   = sol::property([](const lt::torrent_status& ts) { return ts.current_tracker; });
    torrent_status_type["finished_duration"] = sol::property([](const lt::torrent_status& ts) { return ts.finished_duration.count(); });
    torrent_status_type["info_hash"]         = sol::property([](const lt::torrent_status& ts) { return ts.info_hashes; });
    torrent_status_type["is_downloading"]    = sol::property([](const lt::torrent_status& ts) { return ts.state == lt::torrent_status::downloading; });
    torrent_status_type["is_finished"]       = sol::property([](const lt::torrent_status& ts) { return ts.state == lt::torrent_status::finished; });
    torrent_status_type["is_moving"]         = sol::property([](const lt::torrent_status& ts) { return ts.moving_storage; });
    torrent_status_type["is_paused"]         = sol::property([](const lt::torrent_status& ts) { return (ts.flags & lt::torrent_flags::paused) == lt::torrent_flags::paused; });
    torrent_status_type["is_seeding"]        = sol::property([](const lt::torrent_status& ts) { return ts.state == lt::torrent_status::seeding; });
    torrent_status_type["name"]              = sol::property([](const lt::torrent_status& ts) { return ts.name; });
    torrent_status_type["ratio"]             = sol::property([](const lt::torrent_status& ts) { return porla::Utils::Ratio(ts); });
    torrent_status_type["save_path"]         = sol::property([](const lt::torrent_status& ts) { return ts.save_path; });
    torrent_status_type["seeding_duration"]  = sol::property([](const lt::torrent_status& ts) { return ts.seeding_duration.count(); });

    torrent_status_type["size"] = sol::property(
        [](const lt::torrent_status& ts) -> std::optional<std::int64_t>
        {
            if (const auto tf = ts.torrent_file.lock())
            {
                return tf->total_size();
            }

            return std::nullopt;
        });

    torrent_status_type["tags"] = sol::property(
        [](const lt::torrent_status& ts)
        {
            auto client_data = ts.handle.userdata().get<TorrentClientData>();

            if (!client_data->tags.has_value())
            {
                client_data->tags = std::unordered_set<std::string>();
            }

            return *client_data->tags;
        },
        [](lt::torrent_status& ts, const std::unordered_set<std::string>& value)
        {
            auto client_data = ts.handle.userdata().get<TorrentClientData>();
            client_data->tags = value;
        });

    lua["package"]["preload"]["torrents"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table torrents = lua.create_table();

        torrents["add"] = [](sol::this_state s, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            lt::add_torrent_params p;
            p.userdata = lt::client_data_t(new TorrentClientData());

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

                if (options.session.Torrents().find(p.ti->info_hashes()) != options.session.Torrents().end())
                {
                    BOOST_LOG_TRIVIAL(error) << "Torrent already in session";
                    return;
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

            options.session.AddTorrent(p);
        };

        torrents["errors"] = lua.create_table();
        torrents["errors"]["tracker_failure"] = lt::errors::tracker_failure;

        sol::table flags = lua.create_table();
        flags["seed_mode"]             = static_cast<std::uint64_t>(lt::torrent_flags::seed_mode);
        flags["upload_mode"]           = static_cast<std::uint64_t>(lt::torrent_flags::upload_mode);
        flags["share_mode"]            = static_cast<std::uint64_t>(lt::torrent_flags::share_mode);
        flags["apply_ip_filter"]       = static_cast<std::uint64_t>(lt::torrent_flags::apply_ip_filter);
        flags["paused"]                = static_cast<std::uint64_t>(lt::torrent_flags::paused);
        flags["auto_managed"]          = static_cast<std::uint64_t>(lt::torrent_flags::auto_managed);
        flags["duplicate_is_error"]    = static_cast<std::uint64_t>(lt::torrent_flags::duplicate_is_error);
        flags["update_subscribe"]      = static_cast<std::uint64_t>(lt::torrent_flags::update_subscribe);
        flags["super_seeding"]         = static_cast<std::uint64_t>(lt::torrent_flags::super_seeding);
        flags["sequential_download"]   = static_cast<std::uint64_t>(lt::torrent_flags::sequential_download);
        flags["stop_when_ready"]       = static_cast<std::uint64_t>(lt::torrent_flags::stop_when_ready);
        flags["override_trackers"]     = static_cast<std::uint64_t>(lt::torrent_flags::override_trackers);
        flags["override_web_seeds"]    = static_cast<std::uint64_t>(lt::torrent_flags::override_web_seeds);
        flags["need_save_resume"]      = static_cast<std::uint64_t>(lt::torrent_flags::need_save_resume);
        flags["disable_dht"]           = static_cast<std::uint64_t>(lt::torrent_flags::disable_dht);
        flags["disable_lsd"]           = static_cast<std::uint64_t>(lt::torrent_flags::disable_lsd);
        flags["disable_pex"]           = static_cast<std::uint64_t>(lt::torrent_flags::disable_pex);
        flags["no_verify_files"]       = static_cast<std::uint64_t>(lt::torrent_flags::no_verify_files);
        flags["default_dont_download"] = static_cast<std::uint64_t>(lt::torrent_flags::default_dont_download);
        flags["i2p_torrent"]           = static_cast<std::uint64_t>(lt::torrent_flags::i2p_torrent);
        flags["all"]                   = static_cast<std::uint64_t>(lt::torrent_flags::all);
        torrents["flags"] = flags;

        torrents["has"] = [](sol::this_state s, const sol::object& arg)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            if (arg.is<std::shared_ptr<lt::torrent_info>>())
            {
                const auto ti = arg.as<std::shared_ptr<lt::torrent_info>>();
                return options.session.Torrents().find(ti->info_hashes()) != options.session.Torrents().end();
            }

            return false;
        };

        torrents["list"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            std::vector<lt::torrent_status> ret;

            for (const auto& [ info_hash, handle ] : options.session.Torrents())
            {
                ret.emplace_back(handle.status());
            }

            return ret;
        };

        torrents["move"] = [](const lt::torrent_status& t, const sol::table& args)
        {
            if (t.handle.is_valid())
            {
                t.handle.move_storage(args["path"]);
            }
        };

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

        torrents["pause"] = [](const lt::torrent_status& ts)
        {
            if (ts.handle.is_valid())
            {
                ts.handle.pause();
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Torrent not valid";
            }
        };

        torrents["peers"] = lua.create_table();
        torrents["peers"]["list"] = [](const lt::torrent_status& ts)
        {
            if (!ts.handle.is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Invalid torrent handle";
            }

            std::vector<lt::peer_info> peers;
            ts.handle.get_peer_info(peers);
            return peers;
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

            p["download_limit"]  = ts.handle.download_limit();
            p["flags"]           = static_cast<std::uint64_t>(ts.handle.flags());
            p["max_connections"] = ts.handle.max_connections();
            p["max_uploads"]     = ts.handle.max_uploads();
            p["upload_limit"]    = ts.handle.upload_limit();

            return p;
        };

        torrents["properties"]["set"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            if (!ts.handle.is_valid())
            {
                return;
            }

            if (args["download_limit"].valid())  ts.handle.set_download_limit(args["download_limit"]);
            if (args["flags"].valid())           ts.handle.set_flags(static_cast<lt::torrent_flags_t>(args["flags"].get<std::uint64_t>()));
            if (args["max_connections"].valid()) ts.handle.set_max_connections(args["max_connections"]);
            if (args["max_uploads"].valid())     ts.handle.set_max_uploads(args["max_uploads"]);
            if (args["upload_limit"].valid())    ts.handle.set_upload_limit(args["upload_limit"]);
        };

        torrents["reannounce"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            if (!ts.handle.is_valid())
            {
                return;
            }

            int                    seconds = 0;
            int                    index   = -1;

            if (args["seconds"].is<int>())       seconds = args["seconds"];
            if (args["tracker_index"].is<int>()) index   = args["tracker_index"];

            ts.handle.force_reannounce(seconds, index);
        };

        torrents["remove"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            bool remove_files = false;
            if (args["remove_files"].valid()) { remove_files = args["remove_files"].get<bool>(); }

            options.session.Remove(ts.info_hashes, remove_files);
        };

        torrents["resume"] = [](sol::this_state s, const lt::torrent_status& ts)
        {
            sol::state_view lua{s};

            if (ts.handle.is_valid())
            {
                ts.handle.resume();
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Torrent not valid";
            }
        };

        torrents["trackers"] = lua.create_table();
        torrents["trackers"]["list"] = [](const lt::torrent_status& ts)
        {
            if (!ts.handle.is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Invalid torrent handle";
            }

            return ts.handle.trackers();
        };

        return torrents;
    };
}
