#pragma once

#include <memory>

#include <libtorrent/torrent_status.hpp>
#include <sol/sol.hpp>

#include "../../query/pql.hpp"

namespace porla::Lua::Types
{
    class PoQuery
    {
    public:
        static void Register(sol::state& lua);

        explicit PoQuery(const Query::Filter& filter);
        bool Includes(const lt::torrent_status& ts);

    private:
        Query::Filter m_filter;
    };
}
