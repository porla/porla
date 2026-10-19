#include "posessionsiterator.hpp"

#include "posessionhandle.hpp"

#include "../../sessions/session.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoSessionsIterator;

std::shared_ptr<PoSessionHandle> PoSessionsIterator::operator()()
{
    if (m_next >= m_sessions.size())
    {
        return nullptr;
    }

    return std::make_shared<PoSessionHandle>(m_sessions[m_next++]);
}
