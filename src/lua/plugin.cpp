#include "plugin.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <utility>

#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/log/trivial.hpp>
#include <libtorrent/session_stats.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "globals.hpp"
#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "../config.hpp"
#include "../cron.hpp"
#include "../curlmulti.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

namespace
{
    static const auto lt_session_metrics = lt::session_stats_metrics();

    constexpr const char* PluginEntryPoint = "plugin.lua";
    constexpr const char* ConfigRegistryKey = "plugin_config";

    bool IsSuspended(const sol::thread& thread)
    {
        lua_State* L = thread.state();
        return L != nullptr && lua_status(L) == LUA_YIELD;
    }

    // Never blindly construct a sol::error from a failed result - Lua errors are not
    // guaranteed to be strings, and error() with a table would otherwise blow up here.
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

struct CoroutineState
{
    sol::thread thread;
    std::string origin; // "init", "destroy", ... purely for diagnostics
};

struct Plugin::State : public std::enable_shared_from_this<Plugin::State>
{
    struct EventSubscription
    {
        std::size_t             id;
        sol::protected_function callback;
    };

    // Returned to Lua from cron(...). Weak, like EventConnection.
    struct CronHandle
    {
        std::weak_ptr<State> state;
        std::size_t          id = 0;

        void Cancel()
        {
            if (auto s = state.lock()) s->RemoveCronSchedule(id);
            state.reset();
        }
    };

    // Returned to Lua from porla.on(...). Holds a weak ref so :disconnect()
    // is safe even if the plugin is already gone.
    struct EventConnection
    {
        std::weak_ptr<State> state;
        std::size_t          id = 0;

        void Disconnect()
        {
            if (auto s = state.lock()) s->RemoveEventListener(id);
            state.reset(); // idempotent
        }
    };

    PluginLoadOptions                        load_options;
    sol::state                               lua;
    sol::table                               tbl;
    std::optional<Plugin::Meta>              meta;
    std::map<std::string, std::vector<char>> files; // populated by LoadFromArchive

    std::vector<CoroutineState>              active_coroutines;
    boost::asio::steady_timer                coroutine_timer;
    bool                                     timer_armed = false;

    Plugin::UnloadCallback                                unload_callback;
    std::optional<std::chrono::steady_clock::time_point>  unload_deadline;
    bool                                                  destroy_called = false;
    bool                                                  unloading      = false;
    bool                                                  unloaded       = false;

    // signal connections
    std::size_t                                               m_next_event_id = 1;
    std::map<std::string, std::vector<EventSubscription>>     m_event_callbacks;
    std::map<std::string, boost::signals2::scoped_connection> m_signal_connections;

    // cron things
    std::size_t                                          m_next_cron_id = 1;
    std::map<std::size_t, std::shared_ptr<CronSchedule>> m_cron_schedules;

    State(const PluginLoadOptions& opts, const std::optional<std::string>& config)
        : load_options(opts)
        , coroutine_timer(opts.io)
    {
        // Configure in-place rather than assigning a moved-from sol::state.
        ConfigureLuaState(config);
    }

    std::string Name() const
    {
        if (meta && meta->name) return *meta->name;
        return "<unnamed plugin>";
    }

    sol::object AddCronSchedule(const std::string& expr, sol::protected_function callback)
    {
        if (!callback.valid())
        {
            BOOST_LOG_TRIVIAL(warning) << Name() << ": cron(...) called without a function";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        if (unloading || unloaded)
            return sol::make_object(lua.lua_state(), sol::lua_nil);

        const std::size_t    id   = m_next_cron_id++;
        std::weak_ptr<State> weak = weak_from_this();

        std::shared_ptr<CronSchedule> sched;
        try
        {
            sched = CronSchedule::Create(
                load_options.io,
                expr,
                [weak, callback = std::move(callback)]()
                {
                    auto self = weak.lock();
                    if (!self || self->unloading || self->unloaded) return;
                    // Route through the coroutine machinery: cron handlers may
                    // sleep()/do IO, and Unload() will wait for them.
                    self->SpawnCoroutine("cron", callback);
                });
        }
        catch (const cron::bad_cronexpr& err)
        {
            BOOST_LOG_TRIVIAL(error)
                << Name() << ": invalid cron expression '" << expr << "': " << err.what();
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        m_cron_schedules.emplace(id, std::move(sched));
        return sol::make_object(lua.lua_state(), CronHandle{ weak, id });
    }

    void RemoveCronSchedule(std::size_t id)
    {
        auto it = m_cron_schedules.find(id);
        if (it == m_cron_schedules.end()) return;
        if (it->second) it->second->Cancel();
        m_cron_schedules.erase(it);
    }

    void CancelAllCronSchedules()
    {
        for (auto& [id, sched] : m_cron_schedules)
            if (sched) sched->Cancel();
        m_cron_schedules.clear();
    }

    sol::object CreateCronGlobal()
    {
        return sol::make_object(lua,
            [this](const std::string& expr, sol::protected_function callback)
            {
                return AddCronSchedule(expr, std::move(callback));
            });
    }

    sol::object AddEventListener(const std::string& event, sol::protected_function callback)
    {
        if (!callback.valid())
        {
            BOOST_LOG_TRIVIAL(warning)
                << Name() << ": porla.on('" << event << "', ...) called without a function";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        if (unloading || unloaded)
        {
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        if (!EnsureSignalConnected(event))
        {
            BOOST_LOG_TRIVIAL(warning) << Name() << ": porla.on() for unknown event '" << event << "'";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        const std::size_t id = m_next_event_id++;
        m_event_callbacks[event].push_back(EventSubscription{ id, std::move(callback) });

        return sol::make_object(lua.lua_state(), EventConnection{ weak_from_this(), id });
    }

    void RemoveEventListener(std::size_t id)
    {
        for (auto& [event, subs] : m_event_callbacks)
        {
            subs.erase(
                std::remove_if(subs.begin(), subs.end(),
                    [id](const EventSubscription& s) { return s.id == id; }),
                subs.end());

            // Drop the underlying signal connection once nobody listens anymore.
            if (subs.empty())
            {
                m_signal_connections.erase(event);
            }
        }
    }

    template<typename... Args>
    void DispatchEvent(const std::string& event, const Args&... args)
    {
        if (unloaded) return;

        const auto it = m_event_callbacks.find(event);
        if (it == m_event_callbacks.end() || it->second.empty()) return;

        // Copy: a handler may add/remove listeners (or unload the plugin) mid-dispatch.
        const std::vector<EventSubscription> subs = it->second;

        for (const auto& sub : subs)
        {
            try
            {
                // Reuse the coroutine machinery so handlers may sleep()/do IO,
                // and so Unload() waits for them to finish.
                SpawnCoroutine(event, sub.callback, args...);
            }
            catch (const std::exception& err)
            {
                // Never let an exception escape into Sessions' alert loop.
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

    void DisconnectAllSignals()
    {
        m_signal_connections.clear(); // scoped_connection dtor disconnects each
    }

    // -- Lua state -----------------------------------------------------------

    void ConfigureLuaState(const std::optional<std::string>& config)
    {
        lua.open_libraries(
            sol::lib::base,
            sol::lib::coroutine, // was missing - the whole design is coroutine based
            sol::lib::debug,     // needed for tracebacks on errors
            sol::lib::io,
            sol::lib::math,      // was missing
            sol::lib::os,
            sol::lib::package,
            sol::lib::string,
            sol::lib::table);

        lua.new_usertype<porla::CurlMulti>("CurlMulti", sol::no_constructor);

        lua.new_usertype<CronHandle>(
            "porla.CronSchedule",
            "cancel", &CronHandle::Cancel);

        lua.new_usertype<EventConnection>(
            "porla.EventConnection",
            "disconnect", &EventConnection::Disconnect);

        lua.new_usertype<lt::torrent_handle>(
            "lt.torrent_handle",
            sol::no_constructor,
            "is_valid", &lt::torrent_handle::is_valid,
            "status", [](const lt::torrent_handle& th) { return th.status(); });

        lua.new_usertype<lt::torrent_status>(
            "lt.torrent_status",
            sol::no_constructor,
            "name", sol::readonly(&lt::torrent_status::name));

        lua.globals()["cron"]  = CreateCronGlobal();
        lua.globals()["http"]  = porla::Lua::Globals::Http::Build(lua);
        lua.globals()["porla"] = CreatePorlaGlobal();
        lua.globals()["sleep"] = porla::Lua::Globals::Sleep::Build(lua);

        lua.registry()["curl"]     = load_options.curl_multi;
        lua.registry()["db"]       = porla::Lua::Registry::Sqlite3{.db = load_options.config.db};
        lua.registry()["io"]       = porla::Lua::Registry::BoostIoContext{.io = &load_options.io};
        lua.registry()["sessions"] = porla::Lua::Registry::Sessions{.sessions = load_options.sessions};

        porla::Lua::Types::LtSettingsPack::Register(lua);
        porla::Lua::Types::Session::Register(lua);

        EvaluateConfig(config);
    }

    // The config is evaluated exactly once, at load time, instead of being re-parsed on
    // every porla.config() call. Errors therefore surface during load, not at random.
    void EvaluateConfig(const std::optional<std::string>& config)
    {
        if (!config || config->empty())
        {
            return;
        }

        auto load = [this](const std::string& src) -> sol::object
        {
            sol::load_result chunk = lua.load(src, "plugin config");

            if (!chunk.valid())
            {
                return sol::object{};
            }

            sol::protected_function_result result = chunk.get<sol::protected_function>()();

            if (!result.valid())
            {
                BOOST_LOG_TRIVIAL(error)
                    << "Failed to evaluate plugin config: " << DescribeError(result);
                return sol::object{};
            }

            return result.get<sol::object>();
        };

        // Accept both "return { ... }" and a bare "{ ... }" table literal.
        sol::object value = load(*config);

        if (!value.valid())
        {
            value = load("return " + *config);
        }

        if (!value.valid())
        {
            BOOST_LOG_TRIVIAL(warning) << "Plugin config could not be parsed - porla.config() will return nil";
            return;
        }

        lua.registry()[ConfigRegistryKey] = value;
    }

    sol::table CreatePorlaGlobal()
    {
        sol::table porla = lua.create_table();

        porla["config"] = [](sol::this_state s) -> sol::object
        {
            sol::state_view lua{s};
            return lua.registry()[ConfigRegistryKey];
        };

        porla["on"] = [this](sol::this_state s, const std::string& event, sol::function callback)
        {
            return AddEventListener(event, std::move(callback));
        };

        porla["session_metrics"] = [](sol::this_state s)
        {
            sol::state_view lua{s};
            sol::table metrics_tbl = lua.create_table();

            for (const auto& m : lt_session_metrics)
            {
                metrics_tbl[m.name] = m.type == lt::metric_type_t::counter
                    ? "counter"
                    : "gauge";
            }

            return metrics_tbl;
        };

        porla["sessions"] = [](sol::this_state s, const std::string& name)
        {
            sol::state_view lua{s};

            auto& sessions = lua.registry()["sessions"].get<porla::Lua::Registry::Sessions>().sessions;

            // Bind the range to a named reference. The original called sessions.All()
            // twice; if All() returns by value those are begin()/end() of two different
            // temporaries, which is undefined behaviour.
            const auto& all = sessions.All();

            auto found = std::find_if(
                all.begin(),
                all.end(),
                [&name](const auto& iter) { return iter.second->name == name; });

            if (found == all.end())
            {
                return decltype(found->second){};
            }

            return found->second;
        };

        return porla;
    }

    template<typename... Args>
    bool SpawnCoroutine(std::string origin, const sol::protected_function& fn, Args&&... args)
    {
        if (!fn.valid())
        {
            return false;
        }

        sol::thread th = sol::thread::create(lua.lua_state());
        sol::coroutine co(th.state(), fn);

        if (!co.valid())
        {
            BOOST_LOG_TRIVIAL(error) << Name() << ": could not create coroutine for '" << origin << "'";
            return false;
        }

        {
            sol::protected_function_result result = co(std::forward<Args>(args)...);

            if (!result.valid())
            {
                BOOST_LOG_TRIVIAL(error)
                    << Name() << ": error in '" << origin << "': " << DescribeError(result);
                return false;
            }
        }

        if (!IsSuspended(th))
        {
            return true;
        }

        active_coroutines.push_back(CoroutineState{
            .thread = std::move(th),
            .origin = std::move(origin)});

        ArmTimer();

        return true;
    }

    void PruneCoroutines()
    {
        active_coroutines.erase(
            std::remove_if(
                active_coroutines.begin(),
                active_coroutines.end(),
                [](const CoroutineState& cs) { return !IsSuspended(cs.thread); }),
            active_coroutines.end());
    }

    void ArmTimer()
    {
        if (timer_armed || unloaded)
        {
            return;
        }

        if (active_coroutines.empty() && !unloading)
        {
            return;
        }

        timer_armed = true;

        coroutine_timer.expires_after(load_options.coroutine_poll_interval);
        coroutine_timer.async_wait(
            [weak = weak_from_this()](const boost::system::error_code& ec)
            {
                auto self = weak.lock();

                if (!self)
                {
                    return;
                }

                self->timer_armed = false;

                if (ec)
                {
                    return;
                }

                self->Tick();
            });
    }

    void Tick()
    {
        PruneCoroutines();

        if (unloading)
        {
            if (active_coroutines.empty())
            {
                FinishUnload();
                return;
            }

            if (unload_deadline && std::chrono::steady_clock::now() >= *unload_deadline)
            {
                BOOST_LOG_TRIVIAL(warning)
                    << Name() << ": " << active_coroutines.size()
                    << " coroutine(s) still suspended after the unload timeout - abandoning them";

                for (const auto& cs : active_coroutines)
                {
                    BOOST_LOG_TRIVIAL(debug) << Name() << ": abandoned coroutine from '" << cs.origin << "'";
                }

                active_coroutines.clear();

                FinishUnload();

                return;
            }
        }

        ArmTimer();
    }

    void BeginUnload(Plugin::UnloadCallback callback)
    {
        unload_callback = std::move(callback);

        if (unloaded)
        {
            FireCallback();

            return;
        }

        if (!unloading)
        {
            unloading       = true;
            unload_deadline = std::chrono::steady_clock::now() + load_options.unload_timeout;

            CancelAllCronSchedules();
            DisconnectAllSignals();

            if (!destroy_called && tbl.valid())
            {
                destroy_called = true;

                sol::optional<sol::protected_function> destroy = tbl["destroy"];

                if (destroy && destroy->valid())
                {
                    SpawnCoroutine("destroy", *destroy);
                }
            }
        }

        PruneCoroutines();

        if (active_coroutines.empty())
        {
            FinishUnload();

            return;
        }

        ArmTimer();
    }

    void FinishUnload()
    {
        if (unloaded)
        {
            return;
        }

        unloaded  = true;
        unloading = true;

        CancelTimer();
        FireCallback();
    }

    void FireCallback()
    {
        auto cb = std::exchange(unload_callback, {});

        if (!cb)
        {
            return;
        }

        boost::asio::post(load_options.io, [cb = std::move(cb)]() { cb(); });
    }

    void CancelTimer()
    {
        try
        {
            coroutine_timer.cancel();
        }
        catch (const std::exception& err)
        {
            BOOST_LOG_TRIVIAL(debug) << Name() << ": failed to cancel coroutine timer: " << err.what();
        }
    }

    void CallDestroySync()
    {
        if (destroy_called || !tbl.valid())
        {
            return;
        }

        destroy_called = true;

        sol::optional<sol::protected_function> destroy = tbl["destroy"];

        if (!destroy || !destroy->valid())
        {
            return;
        }

        sol::protected_function_result result = (*destroy)();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error) << Name() << ": error in 'destroy': " << DescribeError(result);
        }
    }
};

std::unique_ptr<Plugin> Plugin::LoadFromArchive(
    const std::vector<char>& buffer,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    BOOST_LOG_TRIVIAL(error) << "Loading plugins from archives is not implemented yet";
    return nullptr;
}

std::unique_ptr<Plugin> Plugin::LoadFromPath(
    const std::filesystem::path& path,
    const std::optional<std::string>& config,
    const PluginLoadOptions& opts)
{
    std::error_code ec;

    const bool is_dir = fs::is_directory(path, ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to stat plugin path " << path << ": " << ec.message();
        return nullptr;
    }

    const fs::path plugin_lua = is_dir ? path / PluginEntryPoint : path;

    if (!fs::is_regular_file(plugin_lua, ec) || ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Plugin entry point not found: " << plugin_lua;
        return nullptr;
    }

    try
    {
        auto state = std::make_shared<State>(opts, config);

        sol::load_result chunk = state->lua.load_file(plugin_lua.string());

        if (!chunk.valid())
        {
            sol::error err = chunk;
            BOOST_LOG_TRIVIAL(error) << "Failed to load plugin " << plugin_lua << ": " << err.what();
            return nullptr;
        }

        // Executed on the main state - it may not yield. It should only build and return
        // the plugin table; anything that sleeps or does IO belongs in init().
        sol::protected_function_result result = chunk.get<sol::protected_function>()();

        if (!result.valid())
        {
            BOOST_LOG_TRIVIAL(error)
                << "Failed to run plugin " << plugin_lua << ": " << DescribeError(result);
            return nullptr;
        }

        if (result.return_count() < 1 || result.get_type() != sol::type::table)
        {
            BOOST_LOG_TRIVIAL(error)
                << "Plugin " << plugin_lua << " did not return a table (got "
                << sol::type_name(state->lua.lua_state(), result.get_type()) << ")";
            return nullptr;
        }

        state->tbl = result.get<sol::table>();

        Meta meta;

        if (auto name = state->tbl.get<sol::optional<std::string>>("name"))
        {
            meta.name = *name;
        }

        if (auto version = state->tbl.get<sol::optional<std::string>>("version"))
        {
            meta.version = *version;
        }

        state->meta = std::move(meta);

        if (!state->meta->name)
        {
            BOOST_LOG_TRIVIAL(warning) << "Plugin " << plugin_lua << " has no name";
        }

        sol::optional<sol::protected_function> init = state->tbl["init"];

        if (init && init->valid())
        {
            state->SpawnCoroutine("init", *init);
        }

        return std::unique_ptr<Plugin>(new Plugin(std::move(state)));
    }
    catch (const std::exception& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to load plugin " << plugin_lua << ": " << err.what();
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

    try
    {
        if (!m_state->unloading)
        {
            BOOST_LOG_TRIVIAL(warning)
                << m_state->Name()
                << ": destroyed without Unload() - 'destroy' will be called synchronously and cannot yield";

            m_state->CallDestroySync();
        }
        else if (!m_state->unloaded)
        {
            BOOST_LOG_TRIVIAL(warning) << m_state->Name() << ": destroyed while an unload was still in progress";
        }

        m_state->PruneCoroutines();

        if (!m_state->active_coroutines.empty())
        {
            BOOST_LOG_TRIVIAL(warning)
                << m_state->Name() << ": discarding " << m_state->active_coroutines.size()
                << " suspended coroutine(s)";
        }

        m_state->unload_callback = nullptr;
        m_state->CancelTimer();
        m_state->CancelAllCronSchedules();
    }
    catch (const std::exception& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Error while destroying plugin: " << err.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unknown error while destroying plugin";
    }

    m_state.reset();
}

std::optional<Plugin::Meta> Plugin::GetMeta() const
{
    return m_state ? m_state->meta : std::nullopt;
}

std::size_t Plugin::ActiveCoroutines() const
{
    if (!m_state)
    {
        return 0;
    }

    m_state->PruneCoroutines();

    return m_state->active_coroutines.size();
}

bool Plugin::IsUnloading() const
{
    return m_state && m_state->unloading;
}

bool Plugin::IsUnloaded() const
{
    return m_state && m_state->unloaded;
}

void Plugin::Unload(UnloadCallback callback)
{
    if (!m_state)
    {
        if (callback)
        {
            callback();
        }

        return;
    }

    m_state->BeginUnload(std::move(callback));
}
