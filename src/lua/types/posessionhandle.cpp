#include "posessionhandle.hpp"

#include "potorrentshandle.hpp"

#include "../../sessions/session.hpp"
#include "../../utils/ltsettings.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoTorrentsHandle;

void PoSessionHandle::Register(sol::state& lua)
{
    lua.new_usertype<PoSessionHandle>(
        "PoSessionHandle",
        sol::no_constructor,
        "id", sol::property(&PoSessionHandle::Id),
        "name", sol::property(&PoSessionHandle::Name),
        "add_dht_node", [](const PoSessionHandle& session, const std::string& host, int port)
        {
            session.Lock()->Libtorrent().add_dht_node(std::make_pair(host, port));
        },
        "apply_settings", [](const PoSessionHandle& session, lt::settings_pack& sp)
        {
            Utils::LibtorrentSettingsPack::UpdateStatic(sp);

            auto s = session.Lock();
            s->Libtorrent().apply_settings(sp);
            s->Persist();
        },
        "get_settings", [](const PoSessionHandle& session)
        {
            return session.Lock()->Libtorrent().get_settings();
        },
        "is_paused", [](const PoSessionHandle& session) { return session.Lock()->Libtorrent().is_paused(); },
        "pause", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().pause(); },
        "resume", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().resume(); },
        "torrents", &PoSessionHandle::Torrents);
}

int PoSessionHandle::Id()
{
    return Lock()->Id();
}

std::string PoSessionHandle::Name()
{
    return Lock()->Name();
}

std::shared_ptr<PoTorrentsHandle> PoSessionHandle::Torrents()
{
    return std::make_shared<PoTorrentsHandle>(m_state);
}

std::shared_ptr<porla::Session> PoSessionHandle::Lock() const
{
    auto state = m_state.lock();

    if (state == nullptr)
    {
        throw sol::error("Failed to lock session state");
    }

    return state;
}
