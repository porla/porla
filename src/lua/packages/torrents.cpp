#include "torrents.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <utility>

#include "../plugins/plugin.hpp"
#include "../../config.hpp"
#include "../../session.hpp"
#include "../../torrentclientdata.hpp"

using porla::Lua::Packages::Torrents;

struct SignalConnection
{
    explicit SignalConnection(boost::signals2::connection c)
        : connection(std::move(c))
    {
    }

    ~SignalConnection()
    {
        BOOST_LOG_TRIVIAL(info) << "Disconnecting signal";
        connection.disconnect();
    }

    boost::signals2::connection connection;
};

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

    torrent_status_type["current_tracker"] = sol::property([](const lt::torrent_status& ts) { return ts.current_tracker; });
    torrent_status_type["info_hash"]       = sol::property([](const lt::torrent_status& ts) { return ts.info_hashes; });
    torrent_status_type["name"]            = sol::property([](const lt::torrent_status& ts) { return ts.name; });
    torrent_status_type["save_path"]       = sol::property([](const lt::torrent_status& ts) { return ts.save_path; });

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

        torrents["on"] = [](sol::this_state s, const std::string& name, const sol::function& callback)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            if (name == "added")
            {
                auto connection = options.session.OnTorrentAdded(
                    [cb = callback](const lt::torrent_status& ts)
                    {
                        try
                        {
                            cb(ts);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "removed")
            {
                auto connection = options.session.OnTorrentRemoved(
                    [cb = callback](const lt::info_hash_t& ih)
                    {
                        try
                        {
                            cb(ih);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            return std::shared_ptr<SignalConnection>();
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

        torrents["remove"] = [](sol::this_state s, const lt::torrent_status& ts, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            bool remove_files = false;
            if (args["remove_files"].valid()) { remove_files = args["remove_files"].get<bool>(); }

            options.session.Remove(ts.info_hashes, remove_files);
        };

        return torrents;
    };
}
