#include "sessions.hpp"

#include <libtorrent/session_stats.hpp>

#include "../pluginstate.hpp"

#include "../types/posessionhandle.hpp"
#include "../types/posessionsiterator.hpp"

#include "../../data/models/sessions.hpp"

using porla::Lua::Types::PoSessionHandle;
using porla::Lua::Types::PoSessionsIterator;

static const auto lt_session_metrics = lt::session_stats_metrics();

sol::object porla::Lua::Packages::Sessions::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("count", [](sol::this_state ts) -> sol::object
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return sol::lua_nil;
        }

        const auto all_sessions    = porla::Data::Models::Sessions::List(state->db);
        const auto loaded_sessions = std::count_if(
            all_sessions.begin(),
            all_sessions.end(),
            [state](const auto s) { return state->sessions.Get(s.id) != nullptr; });

        return sol::make_object(lua, loaded_sessions);
    });

    tbl.set_function("default", [](sol::this_state ts) -> std::shared_ptr<PoSessionHandle>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        const auto default_session = porla::Data::Models::Sessions::GetDefault(state->db);

        if (!default_session)
        {
            return nullptr;
        }

        auto default_session_ptr = state->sessions.Get(default_session->id);

        if (default_session_ptr == nullptr)
        {
            return nullptr;
        }

        return std::make_shared<PoSessionHandle>(default_session_ptr);
    });

    tbl.set_function("get", sol::overload(
        [](sol::this_state ts, int id) -> std::shared_ptr<PoSessionHandle>
        {
            sol::state_view lua(ts);

            auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
            auto state = weak.lock();

            if (state == nullptr)
            {
                return nullptr;
            }

            auto session_ptr = state->sessions.Get(id);

            if (session_ptr == nullptr)
            {
                return nullptr;
            }

            return std::make_shared<PoSessionHandle>(session_ptr);
        },
        [](sol::this_state ts, const std::string name) -> std::shared_ptr<PoSessionHandle>
        {
            sol::state_view lua(ts);

            auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
            auto state = weak.lock();

            if (state == nullptr)
            {
                return nullptr;
            }

            const auto& session_db = Data::Models::Sessions::GetByName(state->db, name);

            if (!session_db)
            {
                return nullptr;
            }

            auto session_ptr = state->sessions.Get(session_db->id);

            if (session_ptr == nullptr)
            {
                return nullptr;
            }

            return std::make_shared<PoSessionHandle>(session_ptr);
        }));

    tbl.set_function("list", [](sol::this_state ts) -> sol::object
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return sol::lua_nil;
        }

        return sol::make_object(lua, PoSessionsIterator(state->sessions.All()));
    });

    tbl.set_function("metrics", [](sol::this_state ts)
    {
        sol::state_view lua(ts);
        sol::table metrics_tbl = lua.create_table();

        for (const auto& m : lt_session_metrics)
        {
            metrics_tbl[m.name] = lua.create_table();
            metrics_tbl[m.name]["type"] = m.type == lt::metric_type_t::counter
                ? "counter"
                : "gauge";
        }

        return metrics_tbl;
    });

    return tbl;
}
