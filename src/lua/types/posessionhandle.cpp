#include "posessionhandle.hpp"

#include "potorrentshandle.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoTorrentsHandle;

void PoSessionHandle::Register(sol::state& lua)
{
    lua.new_usertype<PoSessionHandle>(
        "PoSessionHandle",
        sol::no_constructor,
        "name", sol::property(&PoSessionHandle::Name),
        "torrents", &PoSessionHandle::Torrents);
}

std::string PoSessionHandle::Name()
{
    return m_state.lock()->name;
}

std::shared_ptr<PoTorrentsHandle> PoSessionHandle::Torrents()
{
    return std::make_shared<PoTorrentsHandle>(m_state);
}
