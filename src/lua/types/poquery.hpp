#pragma once

#include <memory>

#include <libtorrent/torrent_status.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::Types
{
    class PoQuery
    {
    public:
        static void Register(sol::state& lua);

        explicit PoQuery(const std::function<bool(const lt::torrent_status&)>& filter);
        bool Includes(const lt::torrent_status& ts);

    private:
        std::function<bool(const lt::torrent_status&)> m_filter;
    };
}
