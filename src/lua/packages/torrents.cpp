#include "torrents.hpp"

#include <libtorrent/torrent_info.hpp>

#include "../plugins/plugin.hpp"
#include "../../session.hpp"

using porla::Lua::Packages::Torrents;

void Torrents::Register(sol::state& lua)
{
    lua["package"]["preload"]["torrents"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table torrents = lua.create_table();

        torrents["load"] = [](const std::string& path)
        {
            return std::make_shared<lt::torrent_info>(path);
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

        return torrents;
    };
}
