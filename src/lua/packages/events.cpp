#include "events.hpp"

#include <set>

#include <boost/signals2.hpp>

#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"
#include "../types/pojson.hpp"
#include "../types/posessionhandle.hpp"

using porla::Lua::LuaState;
using porla::Sessions;

namespace
{
    const std::set<std::string, std::less<>> kEvents =
    {
        "session.added",
        "session.loaded",
        "session.removed",
        "session.stats",
        "session.unloaded",
        "session.updated",
        "torrent.added",
        "torrent.checked",
        "torrent.error",
        "torrent.file_error",
        "torrent.finished",
        "torrent.metadata_received",
        "torrent.move_failed",
        "torrent.moved",
        "torrent.paused",
        "torrent.removed",
        "torrent.resumed",
        "torrent.state_changed",
        "tracker.error",
        "tracker.reply",
        "tracker.warning"
    };

    sol::table ToLua(LuaState& state, const Sessions::SessionStatePtr& session, const Sessions::Event& event)
    {
        sol::table tbl = state.lua.create_table();

        for (const auto& [ k, v ] : event.data.items())
        {
            tbl[k] = porla::Lua::Types::PoJson::ToLua(state.lua.lua_state(), v, 0);
        }

        tbl["name"]       = event.name;
        tbl["session_id"] = event.session_id;

        if (session != nullptr)
        {
            tbl["session"] = std::make_shared<porla::Lua::Types::PoSessionHandle>(session);
        }

        if (event.torrent.is_valid())
        {
            tbl["torrent"] = event.torrent;
        }

        if (event.torrent_info_hash != lt::info_hash_t())
        {
            tbl["info_hash"] = event.torrent_info_hash;
        }

        return tbl;
    }

    sol::table EventCache(LuaState& state, const Sessions::SessionStatePtr& session, const Sessions::Event& event)
    {
        auto registry = state.lua.registry();

        if (registry.get_or<std::uint64_t>("porla.events.last_id", 0) == event.id)
        {
            return registry["porla.events.last"];
        }

        sol::table tbl = ToLua(state, session, event);

        registry["porla.events.last_id"] = event.id;
        registry["porla.events.last"]    = tbl;

        return tbl;
    }
}

struct PoCancellableConnection : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableConnection(std::size_t callback_id, std::size_t connection_id)
        : m_callback_id(callback_id)
        , m_connection_id(connection_id)
    {
    }

    void Cancel(sol::this_state ts) override
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        state->CancelScopedConnection(m_connection_id);
        state->RemoveCallback(m_callback_id);
    }

private:
    std::size_t m_callback_id;
    std::size_t m_connection_id;
};

sol::object porla::Lua::Packages::Events::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("on", [](sol::this_state ts, std::string event, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        if (!kEvents.contains(event))
        {
            throw sol::error("invalid event name: " + event);
        }

        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        std::size_t                        callback_id = state->RegisterCallback(callback, false);
        boost::signals2::scoped_connection connection  = state->sessions.OnEvent(
            event,
            [weak, callback_id](const Sessions::SessionStatePtr& session, const Sessions::Event& evt)
            {
                auto state = weak.lock();
                if (state == nullptr) { return; }

                state->InvokeCallback(callback_id, EventCache(*state, session, evt));
            });

        const auto connection_id = state->RegisterScopedConnection(std::move(connection));

        return std::make_shared<PoCancellableConnection>(callback_id, connection_id);
    });

    return tbl;
}
