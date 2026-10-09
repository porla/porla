#pragma once

#include <map>
#include <memory>
#include <tuple>

#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_status.hpp>

#include "poquery.hpp"

namespace lt = libtorrent;

namespace porla
{
    class Session;
}

namespace porla::Lua::Types
{
    class PoTorrentsIterator
    {
    public:
        explicit PoTorrentsIterator(const std::shared_ptr<Session>& session, std::optional<PoQuery> query)
            : m_weak_session(session)
            , m_query(query)
        {
        }

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> operator()();

    private:
        std::weak_ptr<Session>         m_weak_session;
        std::optional<lt::info_hash_t> m_last_hash;
        std::optional<PoQuery>         m_query;
    };

}
