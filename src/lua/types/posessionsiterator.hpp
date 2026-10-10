#pragma once

#include <map>
#include <memory>

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
        explicit PoSessionsIterator(std::map<int, std::shared_ptr<Session>> sessions)
            : m_sessions(sessions)
            , m_iterator(m_sessions.begin())
        {
        }

        std::shared_ptr<PoSessionHandle> operator()();

    private:
        std::map<int, std::shared_ptr<Session>>                 m_sessions;
        std::map<int, std::shared_ptr<Session>>::const_iterator m_iterator;
    };

}
