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

        std::optional<lt::torrent_handle> Get(const std::string& info_hash);

        std::shared_ptr<PoTorrentsIterator> List();

    private:
        std::weak_ptr<porla::Sessions::SessionState> m_state;
    };
}
