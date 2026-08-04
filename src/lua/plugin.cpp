#include "plugin.hpp"

#include <algorithm>
#include <chrono>
#include <map>
#include <utility>

#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/log/trivial.hpp>
#include <sol/sol.hpp>
#include <sqlite3.h>

#include "globals.hpp"
#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "../config.hpp"
#include "../curlmulti.hpp"
#include "../sessions.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;

using porla::Lua::Plugin;
using porla::Lua::PluginLoadOptions;

namespace
{
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

        // Give every protected call a traceback instead of a bare message.
        {
            sol::optional<sol::protected_function> traceback = lua["debug"]["traceback"];
            if (traceback && traceback->valid())
            {
                sol::protected_function::set_default_handler(*traceback);
            }
        }

        lua.new_usertype<porla::CurlMulti>("CurlMulti", sol::no_constructor);

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
            // Missing key -> nil object, which is exactly what we want to hand back.
            return lua.registry()[ConfigRegistryKey];
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

    // -- Coroutines ----------------------------------------------------------

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
            // Ran to completion synchronously - nothing to track.
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
                // The plugin may already be gone - never touch a raw this here.
                auto self = weak.lock();
                if (!self) return;

                self->timer_armed = false;

                if (ec) return; // cancelled

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

    // -- Unload --------------------------------------------------------------

    void BeginUnload(Plugin::UnloadCallback callback)
    {
        unload_callback = std::move(callback);

        if (unloaded)
        {
            // Already finished - just fire the new callback.
            FireCallback();
            return;
        }

        if (!unloading)
        {
            unloading       = true;
            unload_deadline = std::chrono::steady_clock::now() + load_options.unload_timeout;

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
        if (unloaded) return;

        unloaded  = true;
        unloading = true;

        CancelTimer();
        FireCallback();
    }

    void FireCallback()
    {
        auto cb = std::exchange(unload_callback, {});
        if (!cb) return;

        // Posted rather than called inline: the owner will typically delete the Plugin
        // from this callback, and we must not be inside one of our own handlers when
        // that happens.
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

    // Best-effort fallback used when the Plugin is destroyed without Unload(). Runs on the
    // main thread, so `destroy` cannot yield - a yield attempt surfaces as a Lua error
    // rather than a crash.
    void CallDestroySync()
    {
        if (destroy_called || !tbl.valid()) return;

        destroy_called = true;

        sol::optional<sol::protected_function> destroy = tbl["destroy"];

        if (!destroy || !destroy->valid()) return;

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
    if (!m_state) return;

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
    }
    catch (const std::exception& err)
    {
        BOOST_LOG_TRIVIAL(error) << "Error while destroying plugin: " << err.what();
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unknown error while destroying plugin";
    }

    // Any handler still holding a weak_ptr to the state will now find it expired.
    m_state.reset();
}

std::optional<Plugin::Meta> Plugin::GetMeta() const
{
    return m_state ? m_state->meta : std::nullopt;
}

std::size_t Plugin::ActiveCoroutines() const
{
    if (!m_state) return 0;

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
        if (callback) callback();
        return;
    }

    m_state->BeginUnload(std::move(callback));
}
