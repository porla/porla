#include "../globals.hpp"

#include <chrono>
#include <boost/asio.hpp>
#include <libtorrent/session_stats.hpp>

#include "../../data/models/plugins.hpp"
#include "../pluginstate.hpp"

namespace lt = libtorrent;

using porla::Lua::Globals::Porla;

static const auto lt_session_metrics = lt::session_stats_metrics();

sol::object Porla::Build(sol::state& lua)
{
    sol::table porla = lua.create_table();

    porla["config"] = [](sol::this_state s) -> sol::object
    {
        sol::state_view lua(s);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return sol::nil; }

        auto p = Data::Models::Plugins::GetById(state->db, state->plugin_id);

        if (!p) { return sol::nil; }
        if (!p->config) { return sol::nil; };

        sol::load_result chunk = lua.load(p->config.value(), "plugin.config.lua");

        if (!chunk.valid())
        {
            return sol::nil;
        }

        sol::protected_function_result result = chunk.get<sol::protected_function>()();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to evaluate plugin config: " << result.get<std::string>();
            return sol::nil;
        }

        return result.get<sol::object>();
    };

    porla["on"] = [](sol::this_state s, const std::string& event, sol::protected_function callback)
    {
        sol::state_view lua{s};

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr) { return sol::nil; }

        std::size_t                        callback_id = state->next_id++;
        boost::signals2::scoped_connection conn;
        std::size_t                        conn_id = state->next_id++;

        if (event == "torrent.added")
        {
            state->callbacks[callback_id] = callback;

            conn = state->sessions.OnTorrentAdded(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    sol::protected_function callback = std::move(it->second);

                    state->callbacks.erase(callback_id);

                    callback();
                });
        }
        else
        {
            return sol::nil;
        }

        state->signals[conn_id] = std::move(conn);

        return sol::nil;
    };

    porla["session_metrics"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table metrics_tbl = lua.create_table();

        for (const auto& m : lt_session_metrics)
        {
            metrics_tbl[m.name] = lua.create_table();
            metrics_tbl[m.name]["type"] = m.type == lt::metric_type_t::counter
                ? "counter"
                : "gauge";
        }

        return metrics_tbl;
    };

    porla["sessions"] = [](sol::this_state s)
    {
    };

    porla["unix_time"] = []()
    {
        const auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
    };

    return porla;
}
