#include "plugin.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <utility>

#include <boost/asio/post.hpp>
#include <boost/log/trivial.hpp>
#include <libtorrent/session_stats.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "globals.hpp"
#include "packages/sessions.hpp"
#include "packages/timers.hpp"
#include "pluginsource.hpp"
#include "pluginstate.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "types/posessionhandle.hpp"
#include "types/potorrentshandle.hpp"

#include "../config.hpp"
#include "../curlmulti.hpp"
#include "../data/models/sessions.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

namespace
{
    static const auto lt_session_metrics = lt::session_stats_metrics();

    constexpr const char* ConfigRegistryKey = "plugin_config";

    // Formats arguments the way stock print() does - each one run through
    // tostring (so __tostring / __name are honored), separated by tabs.
    std::string Concat(lua_State* L, const sol::variadic_args& args)
    {
        std::string line;

        for (const auto& arg : args)
        {
            std::size_t len = 0;

            // Pushes the string representation; indices in `args` are absolute,
            // so they survive the push.
            const char* str = luaL_tolstring(L, arg.stack_index(), &len);

            if (!line.empty()) line += '\t';
            line.append(str, len);

            lua_pop(L, 1);
        }

        return line;
    }

    std::string DescribeError(const sol::protected_function_result& result)
    {
        if (result.valid() || result.return_count() < 1)
        {
            return "unknown error";
        }

        sol::object value = result.get<sol::object>();

        if (value.is<std::string>())
        {
            return value.as<std::string>();
        }

        return "non-string error value of type "
            + std::string(sol::type_name(result.lua_state(), value.get_type()));
    }
}

struct Plugin::State : public std::enable_shared_from_this<Plugin::State>
{
    struct EventSubscription
    {
        std::size_t             id;
        sol::protected_function callback;
    };

    struct Subscription
    {
        std::weak_ptr<State> state;
        std::size_t          id = 0;

        void Cancel()
        {
            if (auto s = state.lock()) s->RemoveSubscription(id);
            state.reset(); // idempotent
        }
    };

    PluginLoadOptions                        load_options;
    PluginSource                             source;
    sol::state                               lua;
    sol::table                               tbl;
    std::optional<Plugin::Meta>              meta;

    std::vector<std::function<void()>> dtors;

    std::size_t                                               m_next_id = 1;

    using EventSubscriptions = std::shared_ptr<const std::vector<EventSubscription>>;

    std::map<std::string, EventSubscriptions>                 m_event_callbacks;
    std::map<std::string, boost::signals2::scoped_connection> m_signal_connections;

    State(const PluginLoadOptions& opts, const std::optional<std::string>& config)
        : load_options(opts)
    {
        ConfigureLuaState(config);
    }

    std::string Name() const
    {
        if (meta && meta->name) return *meta->name;
        return "<unnamed plugin>";
    }

    // Cancels a cron schedule or an event listener by id. Ids are unique across
    // both, so a single Subscription handle can address either.
    void RemoveSubscription(std::size_t id)
    {
        // Otherwise an event listener. At most one event holds it; drop the
        // underlying signal connection once its last listener goes away.
        for (auto& [event, subs] : m_event_callbacks)
        {
            if (!subs) continue;

            const auto matches = [id](const EventSubscription& s) { return s.id == id; };

            if (std::none_of(subs->begin(), subs->end(), matches)) continue;

            // Rebuild rather than erase in place - a dispatch may be iterating
            // the current vector right now.
            auto next = std::make_shared<std::vector<EventSubscription>>();
            next->reserve(subs->size() - 1);

            std::remove_copy_if(subs->begin(), subs->end(), std::back_inserter(*next), matches);

            if (next->empty()) m_signal_connections.erase(event);

            subs = std::move(next);
            return;
        }
    }

    void CancelAllSubscriptions()
    {
        for (auto& dtor : dtors)
        {
            dtor();
        }

        dtors.clear();

        m_signal_connections.clear();
    }

    sol::object AddEventListener(const std::string& event, sol::protected_function callback)
    {
        if (!callback.valid())
        {
            BOOST_LOG_TRIVIAL(warning)
                << Name() << ": porla.on('" << event << "', ...) called without a function";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        if (!EnsureSignalConnected(event))
        {
            BOOST_LOG_TRIVIAL(warning) << Name() << ": porla.on() for unknown event '" << event << "'";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        const std::size_t id = m_next_id++;

        auto& subs = m_event_callbacks[event];
        auto  next = subs
            ? std::make_shared<std::vector<EventSubscription>>(*subs)
            : std::make_shared<std::vector<EventSubscription>>();

        next->push_back(EventSubscription{ id, std::move(callback) });

        subs = std::move(next);

        return sol::make_object(lua.lua_state(), Subscription{ weak_from_this(), id });
    }

    template<typename... Args>
    void DispatchEvent(const std::string& event, const Args&... args)
    {
        const auto it = m_event_callbacks.find(event);
        if (it == m_event_callbacks.end() || !it->second || it->second->empty()) return;

        const EventSubscriptions subs = it->second;

        for (const auto& sub : *subs)
        {
            try
            {
                sub.callback(args...);
            }
            catch (const std::exception& err)
            {
                BOOST_LOG_TRIVIAL(error)
                    << Name() << ": exception dispatching '" << event << "': " << err.what();
            }
        }
    }

    bool EnsureSignalConnected(const std::string& event)
    {
        if (m_signal_connections.count(event)) return true;

        auto& sessions = load_options.sessions;

        using SessionPtr = std::shared_ptr<porla::Sessions::SessionState>;

        auto handle_slot = [](std::weak_ptr<State> weak, std::string ev)
        {
            return [weak = std::move(weak), ev = std::move(ev)]
                (const SessionPtr&, const lt::torrent_handle& th)
            {
                if (auto self = weak.lock()) self->DispatchEvent(ev, th);
            };
        };

        const std::weak_ptr<State> weak = weak_from_this();
        boost::signals2::scoped_connection conn;

        if      (event == "torrent.added")    conn = sessions.OnTorrentAdded   (handle_slot(weak, event));
        else if (event == "torrent.finished") conn = sessions.OnTorrentFinished(handle_slot(weak, event));
        else if (event == "torrent.paused")   conn = sessions.OnTorrentPaused  (handle_slot(weak, event));
        else if (event == "torrent.resumed")  conn = sessions.OnTorrentResumed (handle_slot(weak, event));
        else if (event == "storage.moved")    conn = sessions.OnStorageMoved   (handle_slot(weak, event));
        else if (event == "torrent.removed")
        {
            conn = sessions.OnTorrentRemoved(
                [weak](const SessionPtr&, const lt::info_hash_t& hash)
                {
                    if (auto self = weak.lock()) self->DispatchEvent("torrent.removed", hash);
                });
        }
        else if (event == "torrent.file_error")
        {
            conn = sessions.OnTorrentFileError(
                [weak](const SessionPtr&, const porla::Sessions::TorrentFileErrorEvent& ev)
                {
                    if (auto self = weak.lock())
                        self->DispatchEvent("torrent.file_error", ev.torrent, ev.file);
                });
        }
        else if (event == "state.update")
        {
            conn = sessions.OnStateUpdate(
                [weak](const SessionPtr&, const std::vector<lt::torrent_status>& statuses)
                {
                    if (auto self = weak.lock())
                        self->DispatchEvent("state.update", sol::as_table(statuses));
                });
        }
        else if (event == "session.stats")
        {
            conn = sessions.OnSessionStats(
                [weak](const SessionPtr& session, const lt::span<const int64_t>& stats)
                {
                    auto self = weak.lock();
                    if (!self) return;

                    sol::table translated = self->lua.create_table();

                    for (const auto& m : lt_session_metrics)
                    {
                        translated[m.name] = stats[m.value_index];
                    }

                    self->DispatchEvent("session.stats", session, translated);
                });
        }
        else
        {
            return false;
        }

        m_signal_connections.emplace(event, std::move(conn));

        return true;
    }

    void ConfigureLuaState(const std::optional<std::string>& config)
    {
        lua.open_libraries(
            sol::lib::base,
            sol::lib::debug,
            sol::lib::io,
            sol::lib::math,
            sol::lib::os,
            sol::lib::package,
            sol::lib::string,
            sol::lib::table);

        Types::LtAnnounceEndpoint::Register(lua);
        Types::LtAnnounceEntry::Register(lua);
        Types::LtAnnounceInfohash::Register(lua);
        Types::LtOpenFileState::Register(lua);
        Types::LtPeerInfo::Register(lua);
        Types::LtSettingsPack::Register(lua);
        Types::LtTorrentHandle::Register(lua);
        Types::LtTorrentStatus::Register(lua);

        // Porla wrapper types
        Types::PoSessionHandle::Register(lua);
        Types::PoTorrentsHandle::Register(lua);

        lua["package"]["preload"]["porla_sessions"] = Packages::Sessions::Load;
        lua["package"]["preload"]["porla_timers"]   = Packages::Timers::Load;

        auto state = std::make_shared<LuaState>(load_options.io, load_options.sessions);
        state->app = load_options.http_server;
        state->db  = load_options.db;

        lua.registry()["state"] = state;

        lua.globals()["print"] = [this](sol::this_state s, sol::variadic_args args)
        {
            BOOST_LOG_TRIVIAL(info) << Name() << ": " << Concat(s, args);
        };
    }
};

std::unique_ptr<Plugin> Plugin::Load(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    auto source = PluginSource::Load(path);

    if (!source.has_value())
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin source from " << path;
        return nullptr;
    }

    if (source->sources.find(source->entrypoint) == source->sources.end())
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin entry point (plugin.lua) not found for " << path;
        return nullptr;
    }

    try
    {
        auto state = std::make_shared<State>(opts, config);
        state->source = *source;

        sol::load_result chunk = state->lua.load_buffer(
            state->source.sources.at(state->source.entrypoint).data(),
            state->source.sources.at(state->source.entrypoint).size(),
            state->source.entrypoint);

        if (!chunk.valid())
        {
            sol::error err = chunk;
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
            return nullptr;
        }

        sol::protected_function_result result = chunk.get<sol::protected_function>()();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error)
                << "Failed to run plugin: " << DescribeError(result);
            return nullptr;
        }

        if (result.return_count() < 1 || result.get_type() != sol::type::table)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Plugin did not return a table (got "
                << sol::type_name(state->lua.lua_state(), result.get_type()) << ")";
            return nullptr;
        }

        state->meta = Meta{
            .name    = path.filename(),
            .version = std::nullopt
        };

        state->tbl  = result.get<sol::table>();

        if (auto name = state->tbl.get<sol::optional<std::string>>("name"))
        {
            state->meta->name = *name;
        }

        if (auto version = state->tbl.get<sol::optional<std::string>>("version"))
        {
            state->meta->version = *version;
        }

        sol::optional<sol::protected_function> init = state->tbl["init"];

        if (init && init->valid())
        {
            sol::protected_function_result init_result = (*init)();

            if (!init_result.valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to run plugin initializer: " << DescribeError(init_result);
                return nullptr;
            }
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const std::exception& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin: " << err.what();
    }

    return nullptr;
}

Plugin::Plugin(std::shared_ptr<State> state)
    : m_state(std::move(state))
{
}

Plugin::~Plugin()
{
    if (!m_state)
    {
        return;
    }

    m_state.reset();
}

std::optional<Plugin::Meta> Plugin::GetMeta() const
{
    return m_state ? m_state->meta : std::nullopt;
}

void Plugin::Unload(UnloadCallback callback)
{
    if (m_state)
    {
        sol::optional<sol::protected_function> destroy = m_state->tbl["destroy"];

        if (destroy && destroy->valid())
        {
            (*destroy)();
        }
    }

    if (callback)
    {
        callback();
    }
}
