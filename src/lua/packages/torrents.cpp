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
            }

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

        torrents["pause"] = [](sol::this_state s, const lt::torrent_status& ts)
        {
            sol::state_view lua{s};

            if (ts.handle.is_valid())
            {
                ts.handle.pause();
            }
            else
            {
                BOOST_LOG_TRIVIAL(error) << "Torrent not valid";
            }
        };

        sol::table properties = lua.create_table();
        properties["get"] = [](sol::this_state s, const lt::torrent_status& ts) -> sol::reference
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

        properties["set"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            if (!ts.handle.is_valid())
            {
                return;
            }

            BOOST_LOG_TRIVIAL(info) << args["flags"].get<std::uint64_t>();

            if (args["download_limit"].valid())  ts.handle.set_download_limit(args["download_limit"]);
            if (args["flags"].valid())           ts.handle.set_flags(static_cast<lt::torrent_flags_t>(args["flags"].get<std::uint64_t>(), lt::torrent_flags::all));
            if (args["max_connections"].valid()) ts.handle.set_max_connections(args["max_connections"]);
            if (args["max_uploads"].valid())     ts.handle.set_max_uploads(args["max_uploads"]);
            if (args["upload_limit"].valid())    ts.handle.set_upload_limit(args["upload_limit"]);
        };

        torrents["properties"] = properties;

        torrents["reannounce"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            if (!ts.handle.is_valid())
            {
                return;
            }

            int                    seconds = 0;
            int                    index   = -1;
            lt::reannounce_flags_t flags   = {};

            if (args["seconds"].is<int>())       seconds = args["seconds"];
            if (args["tracker_index"].is<int>()) index   = args["tracker_index"];

            if (args["ignore_min_interval"].is<bool>()
                && args["ignore_min_interval"].get<bool>())
            {
                flags = lt::torrent_handle::ignore_min_interval;
            }

            ts.handle.force_reannounce(seconds, index, flags);
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

        return torrents;
    };
}
