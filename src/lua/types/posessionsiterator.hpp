#pragma once

#include <map>

#include "../../sessions.hpp"

namespace porla::Lua::Types
{
    class PoSessionHandle;

    class PoSessionsIterator
    {
    public:
        explicit PoSessionsIterator(std::map<int, porla::Sessions::SessionStatePtr> sessions)
            : m_sessions(sessions)
            , m_iterator(m_sessions.begin())
        {
        }

        std::shared_ptr<PoSessionHandle> operator()();

    private:
        std::map<int, porla::Sessions::SessionStatePtr>                 m_sessions;
        std::map<int, porla::Sessions::SessionStatePtr>::const_iterator m_iterator;
    };

}