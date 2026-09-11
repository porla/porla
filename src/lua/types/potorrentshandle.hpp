#pragma once

#include <sol/sol.hpp>

#include "../../sessions.hpp"

namespace porla::Lua::Types
{
    class PoTorrentsIterator;

    class PoTorrentsHandle
    {
    public:
        static void Register(sol::state& lua);

        explicit PoTorrentsHandle(std::weak_ptr<porla::Sessions::SessionState> state)
            : m_state(state) {}

        int Count();

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> Get(const lt::info_hash_t& info_hash);

        std::shared_ptr<PoTorrentsIterator> List();

    private:
        std::weak_ptr<porla::Sessions::SessionState> m_state;
    };
}
