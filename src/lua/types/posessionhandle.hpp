#pragma once

#include <memory>

#include <sol/sol.hpp>

#include "../../sessions.hpp"

namespace porla::Lua::Types
{
    class PoTorrentsHandle;

    class PoSessionHandle
    {
    public:
        static void Register(sol::state& lua);

        explicit PoSessionHandle(std::weak_ptr<porla::Sessions::SessionState> state)
            : m_state(state) {}

        std::string Name();
        std::shared_ptr<PoTorrentsHandle> Torrents();

    private:
        std::weak_ptr<porla::Sessions::SessionState> m_state;
    };
}
