#include "events.hpp"

#include <set>

#include <boost/signals2.hpp>
#include <libtorrent/session_stats.hpp>

#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"
#include "../types/pojson.hpp"
#include "../types/posessionhandle.hpp"

using porla::Lua::LuaState;
using porla::Sessions;

namespace
{
    static const auto lt_session_metrics = lt::session_stats_metrics();

    const std::set<std::string, std::less<>> kSessionEvents =
    {
        "session.added",
        "session.loaded",
        "session.removed",
        "session.unloaded",
        "session.updated"
    };

    sol::table ToLua(LuaState& state, const Sessions::SessionStatePtr& session, const Sessions::Event& event)
    {
        sol::table tbl = state.lua.create_table();

        for (const auto& [ k, v ] : event.data)
        {
            tbl[k] = porla::Lua::Types::PoJson::ToLua(state.lua.lua_state(), v, 0);
        }

        tbl["name"]       = event.name;
        tbl["session_id"] = event.session_id;

        if (session != nullptr)
        {
            tbl["session"] = std::make_shared<porla::Lua::Types::PoSessionHandle>(session);
        }

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
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        std::size_t                        callback_id = state->RegisterCallback(callback, false);
        boost::signals2::scoped_connection connection;

        if (kSessionEvents.contains(event))
        {
            connection = state->sessions.OnEvent(
                [weak, callback_id, event](const Sessions::SessionStatePtr& session, const Sessions::Event& evt)
                {
                    if (evt.name != event) { return; }

                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, ToLua(*state, session, evt));
                });
        }
        else if (event == "session.stats")
        {
            connection = state->sessions.OnSessionStats(
                [weak, callback_id](const auto session, const auto& stats)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    sol::table translated = state->lua.create_table();

                    for (const auto& m : lt_session_metrics)
                    {
                        translated[m.name] = stats[m.value_index];
                    }

                    state->InvokeCallback(callback_id, std::make_shared<Types::PoSessionHandle>(session), translated);
                });
        }
        else if (event == "torrent.added")
        {
            connection = state->sessions.OnTorrentAdded(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, handle);
                });
        }
        else if (event == "torrent.file_error")
        {
            connection = state->sessions.OnTorrentFileError(
                [weak, callback_id](const auto session, const auto& err)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, err.torrent, err.file);
                });
        }
        else if (event == "torrent.finished")
        {
            connection = state->sessions.OnTorrentFinished(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, handle);
                });
        }
        else if (event == "torrent.paused")
        {
            connection = state->sessions.OnTorrentPaused(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, handle);
                });
        }
        else if (event == "torrent.removed")
        {
            connection = state->sessions.OnTorrentRemoved(
                [weak, callback_id](const auto session, const auto& info_hash)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, info_hash);
                });
        }
        else if (event == "torrent.resumed")
        {
            connection = state->sessions.OnTorrentResumed(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, handle);
                });
        }
        else if (event == "torrent.storage_moved")
        {
            connection = state->sessions.OnStorageMoved(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    state->InvokeCallback(callback_id, handle);
                });
        }
        else
        {
            state->RemoveCallback(callback_id);
            return nullptr;
        }

        const auto connection_id = state->RegisterScopedConnection(std::move(connection));

        return std::make_shared<PoCancellableConnection>(callback_id, connection_id);
    });

    return tbl;
}
