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
        "add_dht_node", [](const PoSessionHandle& session, const std::string& host, int port)
        {
            session.m_state.lock()->session->add_dht_node(std::make_pair(host, port));
        },
        "apply_settings", [](const PoSessionHandle& session, const lt::settings_pack& sp)
        {
            session.m_state.lock()->session->apply_settings(sp);
        },
        "get_settings", [](const PoSessionHandle& session)
        {
            return session.m_state.lock()->session->get_settings();
        },
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
