#include "posessionsiterator.hpp"

#include "posessionhandle.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoSessionsIterator;

std::shared_ptr<PoSessionHandle> PoSessionsIterator::operator()()
{
    if (m_iterator == m_sessions.end())
    {
        return nullptr;
    }

    auto session = m_iterator->second;
    std::advance(m_iterator, 1);
    return std::make_shared<PoSessionHandle>(session);
}
