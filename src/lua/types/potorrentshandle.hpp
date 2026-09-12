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

        void Add(const sol::table& params);

        int Count();

        std::optional<std::tuple<lt::torrent_handle, lt::torrent_status>> Get(const lt::info_hash_t& info_hash);

        std::shared_ptr<PoTorrentsIterator> List();

        void Remove(const lt::info_hash_t& ih);
        void Remove(const lt::torrent_handle& th);

    private:
        std::weak_ptr<porla::Sessions::SessionState> m_state;
    };
}
