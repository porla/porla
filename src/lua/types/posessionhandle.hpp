#pragma once

#include <memory>

#include <sol/sol.hpp>

namespace porla
{
    class Session;
}

namespace porla::Lua::Types
{
    class PoTorrentsHandle;

    class PoSessionHandle
    {
    public:
        static void Register(sol::state& lua);

        explicit PoSessionHandle(std::weak_ptr<Session> state)
            : m_state(state) {}

        int Id();
        std::string Name();
        std::shared_ptr<PoTorrentsHandle> Torrents();

    private:
        std::shared_ptr<Session> Lock() const;

        std::weak_ptr<Session> m_state;
    };
}
