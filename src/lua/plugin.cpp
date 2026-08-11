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
#include "registry.hpp"
#include "types.hpp"

#include "../config.hpp"
#include "../cron.hpp"
#include "../curlmulti.hpp"
#include "../data/models/sessions.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

class HttpResponseHandle : std::enable_shared_from_this<HttpResponseHandle>
{
public:
    explicit HttpResponseHandle(uWS::HttpResponse<false>* res)
        : m_res(res)
    {
    }

    ~HttpResponseHandle()
    {
        if (m_res)
        {
            m_res->end();
        }
    }

    void Setup()
    {
        m_res->onAborted([w = weak_from_this()]()
        {
            auto self = w.lock();
            if (!self) { return; }
            self->m_aborted = true;
        });
    }

    void end()
    {
        if (m_aborted) { return; }
        m_res->end();
    }

    void end(std::string_view data)
    {
        if (m_aborted) { return; }
        m_res->end(data);
    }

    void write(std::string_view data)
    {
        if (m_aborted) { return; }
        m_res->write(data);
    }

    void writeStatus(std::string_view status)
    {
        if (m_aborted) { return; }
        m_res->writeStatus(status);
    }

private:
    bool m_aborted = false;
    uWS::HttpResponse<false>* m_res;
};

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
    // main_protected_function, not protected_function: callbacks registered from
    // inside a coroutine would otherwise be anchored to that coroutine's
    // lua_State, and referencing them after the thread is collected is a use
    // after free. main_* re-anchors to the main thread, which outlives us.
    struct EventSubscription
    {
        std::size_t                  id;
        sol::main_protected_function callback;
    };

    // Returned to Lua from both cron(...) and porla.on(...). Holds a weak ref
    // so cancellation stays safe even if the plugin is already gone.
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
    sol::state                               lua;
    sol::table                               tbl;
    std::optional<Plugin::Meta>              meta;
    std::map<std::string, std::vector<char>> files; // populated by LoadFromArchive

    std::vector<std::function<void()>> dtors;

    std::vector<sol::thread>                 active_coroutines;
    boost::asio::steady_timer                coroutine_timer;
    bool                                     timer_armed = false;

    Plugin::UnloadCallback                                unload_callback;
    std::optional<std::chrono::steady_clock::time_point>  unload_deadline;
    bool                                                  destroy_called = false;
    bool                                                  unloading      = false;
    bool                                                  unloaded       = false;

    // Subscriptions (cron schedules + event listeners) share one id space so a
    // single Subscription handle can cancel either.
    std::size_t                                               m_next_id = 1;

    // Copy-on-write: dispatch pins the list with a refcount bump instead of
    // deep-copying every callback (which costs a luaL_ref/unref pair each).
    // Mutations swap in a fresh vector, so a dispatch in progress is unaffected.
    using EventSubscriptions = std::shared_ptr<const std::vector<EventSubscription>>;

    std::map<std::string, EventSubscriptions>                 m_event_callbacks;
    std::map<std::string, boost::signals2::scoped_connection> m_signal_connections;
    std::map<std::size_t, std::shared_ptr<CronSchedule>>      m_cron_schedules;

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

    sol::object AddCronSchedule(const std::string& expr, sol::main_protected_function callback)
    {
        if (!callback.valid())
        {
            BOOST_LOG_TRIVIAL(warning) << Name() << ": cron(...) called without a function";
            return sol::make_object(lua.lua_state(), sol::lua_nil);
        }

        if (unloading || unloaded)
            return sol::make_object(lua.lua_state(), sol::lua_nil);

        const std::size_t    id   = m_next_id++;
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

        return sol::make_object(lua.lua_state(), Subscription{ weak, id });
    }

    // Cancels a cron schedule or an event listener by id. Ids are unique across
    // both, so a single Subscription handle can address either.
    void RemoveSubscription(std::size_t id)
    {
        // Cron schedule?
        if (auto it = m_cron_schedules.find(id); it != m_cron_schedules.end())
        {
            if (it->second) it->second->Cancel();
            m_cron_schedules.erase(it);
            return;
        }

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

        for (auto& [id, sched] : m_cron_schedules)
            if (sched) sched->Cancel();

        m_cron_schedules.clear();
        m_signal_connections.clear();
    }

    sol::object AddEventListener(const std::string& event, sol::main_protected_function callback)
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
        if (unloaded) return;

        const auto it = m_event_callbacks.find(event);
        if (it == m_event_callbacks.end() || !it->second || it->second->empty()) return;

        // Pin the list for the duration of the loop: a handler may add or remove
        // listeners (or unload the plugin) mid-dispatch, which swaps the map
        // entry without disturbing what we hold here.
        const EventSubscriptions subs = it->second;

        for (const auto& sub : *subs)
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
                        sol::table t = self->lua.create_table();
                        t["type"]  = m.type == lt::metric_type_t::counter
                            ? "gauge"
                            : "counter";
                        t["value"] = stats[m.value_index];

                        translated[m.name] = t;
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

    // -- Lua state -----------------------------------------------------------

    void ConfigureLuaState(const std::optional<std::string>& config)
    {
        lua.open_libraries(
            sol::lib::base,
            sol::lib::coroutine,
            sol::lib::debug,
            sol::lib::io,
            sol::lib::math,
            sol::lib::os,
            sol::lib::package,
            sol::lib::string,
            sol::lib::table);

        lua.new_usertype<porla::CurlMulti>("CurlMulti", sol::no_constructor);

        lua.new_usertype<uWS::HttpRequest>(
            "uWS.HttpRequest",
            sol::no_constructor);

        lua.new_usertype<HttpResponseHandle>(
            "http_server.Response",
            sol::no_constructor,
            "finish", sol::overload(
                [](HttpResponseHandle& h) { h.end(); },
                [](HttpResponseHandle& h, std::string_view data) { h.end(data); }
            ),
            "write", &HttpResponseHandle::write,
            "writeStatus", &HttpResponseHandle::writeStatus);

        // One handle type for cron + events; keep both verbs Lua scripts call.
        lua.new_usertype<Subscription>(
            "porla.Subscription",
            "cancel",     &Subscription::Cancel,
            "disconnect", &Subscription::Cancel);

        lua.new_usertype<lt::torrent_handle>(
            "lt.torrent_handle",
            sol::no_constructor,
            "is_valid", &lt::torrent_handle::is_valid,
            "status", [](const lt::torrent_handle& th) { return th.status(); });

        lua.new_usertype<lt::torrent_status>(
            "lt.torrent_status",
            sol::no_constructor,
            "name", sol::readonly(&lt::torrent_status::name));

        lua.globals()["cron"] = [this](const std::string& expr, sol::main_protected_function cb)
        {
            return AddCronSchedule(expr, std::move(cb));
        };

        lua.globals()["print"] = [this](sol::this_state s, sol::variadic_args args)
        {
            BOOST_LOG_TRIVIAL(info) << Name() << ": " << Concat(s, args);
        };

        auto http_server = lua.create_table();
        http_server["get"] = [this](const std::string& pattern, sol::main_protected_function func)
        {
            BOOST_LOG_TRIVIAL(trace) << "Attaching HTTP GET handler for " << pattern;

            auto app = lua.registry()["http_server"].get<porla::Lua::Registry::uWebSocketsApp>().app;

            app->get(pattern, [w = weak_from_this(), callback = std::move(func)](uWS::HttpResponse<false>* res, auto req)
            {
                auto self = w.lock();
                if (!self || self->unloading || self->unloaded) return;

                auto response = std::make_shared<HttpResponseHandle>(res);
                response->Setup();

                self->SpawnCoroutine("http_server.get", callback, req, response);
            });

            dtors.emplace_back([w = weak_from_this(), p = pattern]()
            {
                auto self = w.lock();
                if (!self) return;
                self->load_options.http_server->get(p, nullptr);
            });
        };

        lua.globals()["http"]        = porla::Lua::Globals::Http::Build(lua);
        lua.globals()["http_server"] = http_server;

        lua.globals()["porla"] = CreatePorlaGlobal();
        lua.globals()["sleep"] = porla::Lua::Globals::Sleep::Build(lua);

        lua.registry()["curl"]        = load_options.curl_multi;
        lua.registry()["db"]          = porla::Lua::Registry::Sqlite3{.db = load_options.config.db};
        lua.registry()["http_server"] = porla::Lua::Registry::uWebSocketsApp{.app = load_options.http_server};
        lua.registry()["io"]          = porla::Lua::Registry::BoostIoContext{.io = &load_options.io};
        lua.registry()["sessions"]    = porla::Lua::Registry::Sessions{.sessions = load_options.sessions};

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

        porla["on"] = [this](const std::string& event, sol::main_protected_function callback)
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

        porla["sessions"] = [](sol::this_state s, const std::string& name) -> std::shared_ptr<porla::Sessions::SessionState>
        {
            sol::state_view lua{s};

            auto db = lua.registry()["db"].get<porla::Lua::Registry::Sqlite3>().db;

            const auto& sessions = Data::Models::Sessions::List(db);

            auto found = std::find_if(
                sessions.begin(),
                sessions.end(),
                [&name](const auto& iter) { return iter.name == name; });

            if (found == sessions.end())
            {
                return nullptr;
            }

            return lua.registry()["sessions"].get<porla::Lua::Registry::Sessions>().sessions.Get(found->id);
        };

        return porla;
    }

    // Fn is templated so both protected_function and main_protected_function
    // pass through without a re-ref round trip.
    template<typename Fn, typename... Args>
    bool SpawnCoroutine(std::string origin, const Fn& fn, Args&&... args)
    {
        BOOST_LOG_TRIVIAL(debug) << "Spawning coroutine in " << origin;

        if (!fn.valid())
        {
            BOOST_LOG_TRIVIAL(warning) << "Function not valid";
            return false;
        }

        BOOST_LOG_TRIVIAL(trace) << "Creating Lua thread";
        sol::thread th = sol::thread::create(lua);

        BOOST_LOG_TRIVIAL(trace) << "Creating Lua coroutine";
        sol::coroutine co(th.thread_state(), fn);

        if (!co.valid())
        {
            BOOST_LOG_TRIVIAL(error) << Name() << ": could not create coroutine for '" << origin << "'";
            return false;
        }

        {
            BOOST_LOG_TRIVIAL(trace) << "Executing coroutine";

            sol::protected_function_result result = co(std::forward<Args>(args)...);

            if (!result.valid())
            {
                BOOST_LOG_TRIVIAL(error)
                    << Name() << ": error in '" << origin << "': " << DescribeError(result);
                return false;
            }

            BOOST_LOG_TRIVIAL(trace) << "Exiting coroutine result scope";
        }

        if (!IsSuspended(th))
        {
            return true;
        }

        active_coroutines.push_back(std::move(th));

        ArmTimer();

        return true;
    }

    void PruneCoroutines()
    {
        active_coroutines.erase(
            std::remove_if(
                active_coroutines.begin(),
                active_coroutines.end(),
                [](const sol::thread& st) { return !IsSuspended(st); }),
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

            CancelAllSubscriptions();

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
        m_state->CancelAllSubscriptions();
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