#include "../packages.hpp"

#include <set>

#include <boost/signals2.hpp>

#include "../../events.hpp"
#include "../../keyvalueevent.hpp"
#include "../../pluginevent.hpp"
#include "../../presetevent.hpp"
#include "../../sessions/sessionevent.hpp"
#include "../../sessions/torrentevent.hpp"

#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"
#include "../types/pojson.hpp"
#include "../types/posessionhandle.hpp"

using porla::Lua::LuaState;

namespace
{
    const std::set<std::string, std::less<>> kEvents =
    {
        "kv.updated",
        "plugin.added",
        "plugin.loaded",
        "plugin.removed",
        "plugin.unloaded",
        "plugin.updated",
        "preset.added",
        "preset.removed",
        "preset.updated",
        "session.added",
        "session.dht_stats",
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
        "torrent.need_cert",
        "torrent.storage_moved_failed",
        "torrent.storage_moved",
        "torrent.paused",
        "torrent.removed",
        "torrent.resumed",
        "torrent.state_changed",
        "torrent.tracker_error",
        "torrent.tracker_reply",
        "torrent.tracker_warning",
        "torrent.userdata_updated"
    };

    sol::table ToLua(LuaState& state, const porla::Event& event)
    {
        sol::table tbl = state.lua.create_table();

        for (const auto& [ k, v ] : event.data.items())
        {
            if (v.is_null())
            {
                continue;
            }

            tbl[k] = porla::Lua::Types::PoJson::ToLua(state.lua.lua_state(), v, 0);
        }

        tbl["name"] = event.name;

        if (const auto* session_event = dynamic_cast<const porla::SessionEvent*>(&event))
        {
            tbl["session_id"] = session_event->session_id;

            if (const auto session = session_event->session.lock())
            {
                tbl["session"] = std::make_shared<porla::Lua::Types::PoSessionHandle>(session);
            }
        }

        if (const auto* torrent_event = dynamic_cast<const porla::TorrentEvent*>(&event))
        {
            tbl["info_hash"] = torrent_event->info_hash;

            if (torrent_event->torrent_handle.is_valid())
            {
                tbl["torrent"] = torrent_event->torrent_handle;
            }
        }

        if (const auto* kv_event = dynamic_cast<const porla::KeyValueEvent*>(&event))
        {
            tbl["keys"] = sol::as_table(kv_event->keys);
        }

        if (const auto* plugin_event = dynamic_cast<const porla::PluginEvent*>(&event))
        {
            tbl["plugin_id"] = plugin_event->plugin_id;
        }

        if (const auto* preset_event = dynamic_cast<const porla::PresetEvent*>(&event))
        {
            tbl["preset_id"] = preset_event->preset_id;
        }

        return tbl;
    }

    sol::table EventCache(LuaState& state, const porla::Event& event)
    {
        auto registry = state.lua.registry();

        if (registry.get_or<std::uint64_t>("porla.events.last_id", 0) == event.id)
        {
            return registry["porla.events.last"];
        }

        sol::table tbl = ToLua(state, event);

        registry["porla.events.last_id"] = event.id;
        registry["porla.events.last"]    = tbl;

        return tbl;
    }
}

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

        auto weak = LuaState::WeakFrom(ts);
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        std::size_t                        callback_id = state->RegisterCallback(callback, false);
        boost::signals2::scoped_connection connection  = state->events.On(
            event,
            [weak, callback_id](const porla::Event& event)
            {
                auto state = weak.lock();
                if (state == nullptr) { return; }

                // a plugin never receives plugin events about itself
                if (const auto* plugin_event = dynamic_cast<const porla::PluginEvent*>(&event))
                {
                    if (plugin_event->plugin_id == state->plugin_id)
                    {
                        return;
                    }
                }

                state->InvokeCallback(callback_id, EventCache(*state, event));
            });

        const auto connection_id = state->RegisterScopedConnection(std::move(connection));

        return std::make_shared<Types::PoCancellable>([callback_id, connection_id](LuaState& s)
        {
            s.CancelScopedConnection(connection_id);
            s.RemoveCallback(callback_id);
        });
    });

    return tbl;
}
