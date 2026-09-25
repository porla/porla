#pragma once

#include <map>
#include <tuple>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "poquery.hpp"
#include "../../sessions.hpp"

namespace lt = libtorrent;

namespace porla::Lua::Types
{
    class PoTorrentsIterator
    {
    public:
        explicit PoTorrentsIterator(const std::shared_ptr<Sessions::SessionState>& state, std::optional<PoQuery> query)
            : m_state(state)
            , m_query(query)
        {
        }

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> operator()();

    private:
        std::weak_ptr<Sessions::SessionState>                         m_state;
        std::optional<lt::info_hash_t>                                m_last_hash;
        std::optional<PoQuery>                                        m_query;
    };

}
