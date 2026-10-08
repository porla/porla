#pragma once

#include <functional>

#include <libtorrent/add_torrent_params.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class LtAddTorrentParams
    {
    public:
        static void Register(sol::state& lua);

        static lt::add_torrent_params ToParams(
            const sol::object& params,
            const std::function<void(lt::add_torrent_params&)>& prepare);

        static sol::table ToTable(sol::this_state ts, const lt::add_torrent_params& params);
    };
}
