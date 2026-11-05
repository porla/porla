#include "posessionsiterator.hpp"

#include "posessionhandle.hpp"

#include "../../sessions/session.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoSessionsIterator;

std::shared_ptr<PoSessionHandle> PoSessionsIterator::operator()()
{
    while (m_next < m_sessions.size())
    {
        if (auto session = m_sessions[m_next++].lock())
        {
            return std::make_shared<PoSessionHandle>(session);
        }
    }

    return nullptr;
}
