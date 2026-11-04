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
            if (const auto invalid = Utils::LibtorrentSettingsPack::Validate(sp))
            {
                throw sol::error("invalid value for setting '" + *invalid + "'");
            }

            Utils::LibtorrentSettingsPack::UpdateStatic(sp);

            auto s = session.Lock();
            s->Libtorrent().apply_settings(sp);
            s->Persist();
        },
        "get_settings", [](const PoSessionHandle& session)
        {
            return session.Lock()->Libtorrent().get_settings();
        },
        "is_listening", [](const PoSessionHandle& session) { return session.Lock()->Libtorrent().is_listening(); },
        "is_paused", [](const PoSessionHandle& session) { return session.Lock()->Libtorrent().is_paused(); },
        "pause", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().pause(); },
        "resume", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().resume(); },
        "post_dht_stats", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().post_dht_stats(); },
        "post_session_stats", [](const PoSessionHandle& session) { session.Lock()->Libtorrent().post_session_stats(); },
        "torrents", &PoSessionHandle::Torrents,
        sol::meta_function::equal_to,  &PoSessionHandle::operator==,
        sol::meta_function::to_string, &PoSessionHandle::ToString);
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

// compares ownership, not the pointer, so it still works once the session is gone
bool PoSessionHandle::operator==(const PoSessionHandle& other) const
{
    return !m_state.owner_before(other.m_state) && !other.m_state.owner_before(m_state);
}

std::string PoSessionHandle::ToString() const
{
    const auto session = m_state.lock();

    if (session == nullptr)
    {
        return "PoSessionHandle(gone)";
    }

    return "PoSessionHandle(" + std::to_string(session->Id()) + ", \"" + session->Name() + "\")";
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
