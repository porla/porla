#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

namespace porla
{
    class Session;
}

namespace porla::Lua::Types
{
    class PoSessionHandle;

    class PoSessionsIterator
    {
    public:
        // A snapshot in ascending id order. Holds a position, not an iterator into its own
        // container, so it stays valid when sol copies or moves it into userdata.
        explicit PoSessionsIterator(const std::map<int, std::shared_ptr<Session>>& sessions)
        {
            m_sessions.reserve(sessions.size());

            for (const auto& [ _, session ] : sessions)
            {
                m_sessions.push_back(session);
            }
        }

        std::shared_ptr<PoSessionHandle> operator()();

    private:
        std::vector<std::shared_ptr<Session>> m_sessions;
        std::size_t                           m_next = 0;
    };

}
