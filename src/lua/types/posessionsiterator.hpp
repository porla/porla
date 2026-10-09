#pragma once

#include <map>

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
        using SessionPtr = std::shared_ptr<Session>;

        explicit PoSessionsIterator(std::map<int, SessionPtr> sessions)
            : m_sessions(sessions)
            , m_iterator(m_sessions.begin())
        {
        }

        std::shared_ptr<PoSessionHandle> operator()();

    private:
        std::map<int, SessionPtr>                 m_sessions;
        std::map<int, SessionPtr>::const_iterator m_iterator;
    };

}
