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
#include "pluginsource.hpp"
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

class TorrentsIterator
{
public:
    explicit TorrentsIterator(const std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>>& torrents)
        : m_torrents(torrents)
        , m_iterator(m_torrents.begin())
    {
    }

    std::optional<lt::torrent_handle> operator()()
    {
        if (m_iterator == m_torrents.end())
        {
            return std::nullopt;
        }

        auto [ th, _ ] = m_iterator->second;
        std::advance(m_iterator, 1);

        return th;
    }

private:
    std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>> const&          m_torrents;
    std::map<lt::info_hash_t, std::tuple<lt::torrent_handle, lt::torrent_status>>::const_iterator m_iterator;
};

class TorrentsHandle
{
public:
    explicit TorrentsHandle(std::weak_ptr<porla::Sessions::SessionState> state)
        : m_state(state) {}

    int Count()
    {
        return m_state.lock()->torrents.size();
    }

    std::optional<lt::torrent_handle> Get(const std::string& info_hash)
    {
        lt::sha1_hash hash;

        {
            std::stringstream ss(info_hash);
            ss >> hash;
        }

        auto state = m_state.lock();
        auto found = state->torrents.find(lt::info_hash_t(hash));

        if (found == state->torrents.end())
        {
            return std::nullopt;
        }

        auto [ th, _ ] = found->second;

        return th;
    }

    TorrentsIterator List()
    {
        return TorrentsIterator(m_state.lock()->torrents);
    }

private:
    std::weak_ptr<porla::Sessions::SessionState> m_state;
};

class SessionHandle
{
public:
    explicit SessionHandle(std::weak_ptr<porla::Sessions::SessionState> state)
        : m_state(state) {}

    std::string Name()
    {
        return m_state.lock()->name;
    }

    std::shared_ptr<TorrentsHandle> Torrents()
    {
        return std::make_shared<TorrentsHandle>(m_state);
    }

private:
    std::weak_ptr<porla::Sessions::SessionState> m_state;
};

class SessionsIterator
{
public:
    explicit SessionsIterator(std::map<int, porla::Sessions::SessionStatePtr> sessions)
        : m_sessions(sessions)
        , m_iterator(m_sessions.begin())
    {
    }

    std::shared_ptr<SessionHandle> operator()()
    {
        if (m_iterator == m_sessions.end())
        {
            return nullptr;
        }

        auto session = m_iterator->second;
        std::advance(m_iterator, 1);
        return std::make_shared<SessionHandle>(session);
    }

private:
    std::map<int, porla::Sessions::SessionStatePtr>                 m_sessions;
    std::map<int, porla::Sessions::SessionStatePtr>::const_iterator m_iterator;
};

class SessionsHandle
{
public:
    explicit SessionsHandle(sqlite3* db, porla::Sessions& sessions)
        : m_db(db)
        , m_sessions(sessions) {}

    int Count()
    {
        const auto all_sessions    = porla::Data::Models::Sessions::List(m_db);
        const auto loaded_sessions = std::count_if(
            all_sessions.begin(),
            all_sessions.end(),
            [this](const auto s) { return m_sessions.Get(s.id) != nullptr; });

        return loaded_sessions;
    }

    std::shared_ptr<SessionHandle> Default()
    {
        const auto default_session = porla::Data::Models::Sessions::GetDefault(m_db);

        if (!default_session.has_value())
        {
            return nullptr;
        }

        const auto state = m_sessions.Get(default_session->id);

        return std::make_shared<SessionHandle>(state);
    }

    SessionHandle Get(const std::string& name);

    SessionsIterator List()
    {
        return SessionsIterator(m_sessions.All());
    }

private:
    sqlite3*         m_db;
    porla::Sessions& m_sessions;
};

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
    PluginSource                             source;
    sol::state                               lua;
    sol::table                               tbl;
    std::optional<Plugin::Meta>              meta;

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

        lua.new_usertype<SessionHandle>(
            "porla.Session",
            sol::no_constructor,
            "name",     sol::property(&SessionHandle::Name),
            "torrents", &SessionHandle::Torrents);

        lua.new_usertype<SessionsHandle>(
            "porla.Sessions",
            sol::no_constructor,
            "count",   &SessionsHandle::Count,
            "default", &SessionsHandle::Default,
            "list",    &SessionsHandle::List);

        lua.new_usertype<TorrentsHandle>(
            "porla.Torrents",
            sol::no_constructor,
            "count", &TorrentsHandle::Count,
            "get",   &TorrentsHandle::Get,
            "list",  &TorrentsHandle::List);

        lua.new_usertype<lt::torrent_handle>(
            "porla.Torrent",
            sol::no_constructor,
            // add_piece
            // add_tracker
            "add_url_seed",               &lt::torrent_handle::add_url_seed,
            "clear_error",                &lt::torrent_handle::clear_error,
            "clear_peers",                &lt::torrent_handle::clear_peers,
            "clear_piece_deadlines",      &lt::torrent_handle::clear_piece_deadlines,
            // connect_peer
            "download_limit",             &lt::torrent_handle::download_limit,
            // file_priority
            // file_progress
            "file_status",                &lt::torrent_handle::file_status,
            // flags
            "flush_cache",                &lt::torrent_handle::flush_cache,
            "force_dht_announce",         &lt::torrent_handle::force_dht_announce,
            "force_lsd_announce",         &lt::torrent_handle::force_lsd_announce,
            // force_reannounce
            "force_recheck",              &lt::torrent_handle::force_recheck,
            //"get_download_queue",       &lt::torrent_handle::get_download_queue,
            // get_file_priorities
            // get_peer_info
            // get_piece_priorities
            "get_renamed_files",          &lt::torrent_handle::get_renamed_files,
            // get_resume_data
            "have_piece",                 &lt::torrent_handle::have_piece,
            "in_session",                 &lt::torrent_handle::in_session,
            "info_hash",                  &lt::torrent_handle::info_hashes,
            "is_valid",                   &lt::torrent_handle::is_valid,
            "max_connections",            &lt::torrent_handle::max_connections,
            "max_uploads",                &lt::torrent_handle::max_uploads,
            "move_storage",               &lt::torrent_handle::move_storage,
            // "need_save_resume_data"
            "pause",                      &lt::torrent_handle::pause,
            // piece_availability
            //"piece_layers",               &lt::torrent_handle::piece_layers,
            // piece_priority
            // post_download_queue
            "post_file_priorities",       &lt::torrent_handle::post_file_priorities,
            // post_file_progress
            // post_file_status
            // post_peer_info
            "post_piece_availability",    &lt::torrent_handle::post_piece_availability,
            // post_status
            "post_trackers",              &lt::torrent_handle::post_trackers,
            // prioritize_files
            // prioritize_pieces
            "queue_position",             &lt::torrent_handle::queue_position,
            "queue_position_bottom",      &lt::torrent_handle::queue_position_bottom,
            "queue_position_down",        &lt::torrent_handle::queue_position_down,
            "queue_position_set",         &lt::torrent_handle::queue_position_set,
            "queue_position_top",         &lt::torrent_handle::queue_position_top,
            "queue_position_up",          &lt::torrent_handle::queue_position_up,
            // read_piece
            "remove_url_seed",            &lt::torrent_handle::remove_url_seed,
            "rename_file",                &lt::torrent_handle::rename_file,
            // replace_trackers
            // reset_piece_deadline
            "resume",                     &lt::torrent_handle::resume,
            // save_resume_data
            // scrape_tracker
            "set_download_limit",         &lt::torrent_handle::set_download_limit,
            // set_flags
            "set_max_connections",        &lt::torrent_handle::set_max_connections,
            "set_max_uploads",            &lt::torrent_handle::set_max_uploads,
            // set_metadata
            // set_piece_deadline
            // set_sequential_range
            "set_ssl_certificate",        &lt::torrent_handle::set_ssl_certificate,
            "set_ssl_certificate_buffer", &lt::torrent_handle::set_ssl_certificate_buffer,
            "set_upload_limit",           &lt::torrent_handle::set_upload_limit,
            "status",                     [](const lt::torrent_handle& th) { return th.status(); },
            "torrent_file",               &lt::torrent_handle::torrent_file,
            "trackers",                   &lt::torrent_handle::trackers,
            // unset_flags
            "upload_limit",               &lt::torrent_handle::upload_limit,
            "url_seeds",                  &lt::torrent_handle::url_seeds);

        lua.new_usertype<lt::torrent_status>(
            "lt.torrent_status",
            sol::no_constructor,
            "active_duration",        sol::property([](const lt::torrent_status& ts) { return ts.active_duration.count(); }),
            "added_time",             sol::readonly(&lt::torrent_status::added_time),
            "all_time_download",      sol::readonly(&lt::torrent_status::all_time_download),
            "all_time_upload",        sol::readonly(&lt::torrent_status::all_time_upload),
            "announcing_to_dht",      sol::readonly(&lt::torrent_status::announcing_to_dht),
            "announcing_to_lsd",      sol::readonly(&lt::torrent_status::announcing_to_lsd),
            "announcing_to_trackers", sol::readonly(&lt::torrent_status::announcing_to_trackers),
            "block_size",             sol::readonly(&lt::torrent_status::block_size),
            "completed_time",         sol::readonly(&lt::torrent_status::completed_time),
            "connect_candidates",     sol::readonly(&lt::torrent_status::connect_candidates),
            "connections_limit",      sol::readonly(&lt::torrent_status::connections_limit),
            "current_tracker",        sol::readonly(&lt::torrent_status::current_tracker),
            "distributed_copies",     sol::readonly(&lt::torrent_status::distributed_copies),
            "down_bandwidth_queue",   sol::readonly(&lt::torrent_status::down_bandwidth_queue),
            "download_limit",         sol::readonly(&lt::torrent_status::download_limit),
            "download_payload_rate",  sol::readonly(&lt::torrent_status::download_payload_rate),
            "download_rate",          sol::readonly(&lt::torrent_status::download_rate),
            // flags
            "finished_duration",      sol::property([](const lt::torrent_status& ts) { return ts.finished_duration.count(); }),
            "handle",                 sol::readonly(&lt::torrent_status::handle),
            "has_incoming",           sol::readonly(&lt::torrent_status::has_incoming),
            "has_metadata",           sol::readonly(&lt::torrent_status::has_metadata),
            "info_hash",              sol::readonly(&lt::torrent_status::info_hashes),
            "is_finished",            sol::readonly(&lt::torrent_status::is_finished),
            "is_seeding",             sol::readonly(&lt::torrent_status::is_seeding),
            "last_download",          sol::property([](const lt::torrent_status& ts) { return std::chrono::duration_cast<std::chrono::seconds>(ts.last_download.time_since_epoch()).count(); }),
            "last_seen_complete",     &lt::torrent_status::last_seen_complete,
            "last_upload",            sol::property([](const lt::torrent_status& ts) { return std::chrono::duration_cast<std::chrono::seconds>(ts.last_upload.time_since_epoch()).count(); }),
            "list_peers",             sol::readonly(&lt::torrent_status::list_peers),
            "list_seeds",             sol::readonly(&lt::torrent_status::list_seeds),
            "moving_storage",         sol::readonly(&lt::torrent_status::moving_storage),
            "name",                   sol::readonly(&lt::torrent_status::name),
            // need_save_resume_data
            "next_announce",          sol::property([](const lt::torrent_status& ts) { return std::chrono::duration_cast<std::chrono::seconds>(ts.next_announce).count(); }),
            "num_complete",           sol::readonly(&lt::torrent_status::num_complete),
            "num_connections",        sol::readonly(&lt::torrent_status::num_connections),
            "num_incomplete",         sol::readonly(&lt::torrent_status::num_incomplete),
            "num_peers",              sol::readonly(&lt::torrent_status::num_peers),
            "num_pieces",             sol::readonly(&lt::torrent_status::num_pieces),
            "num_seeds",              sol::readonly(&lt::torrent_status::num_seeds),
            "num_uploads",            sol::readonly(&lt::torrent_status::num_uploads),
            // pieces
            "progress",               sol::readonly(&lt::torrent_status::progress),
            "queue_position",         sol::readonly(&lt::torrent_status::queue_position),
            // renamed_files
            "save_path",              sol::readonly(&lt::torrent_status::save_path),
            "seed_rank",              sol::readonly(&lt::torrent_status::seed_rank),
            "seeding_duration",      sol::property([](const lt::torrent_status& ts) { return ts.seeding_duration.count(); }),
            // state
            // storage_mode
            "torrent_file",           sol::readonly(&lt::torrent_status::torrent_file),
            "total",                  sol::readonly(&lt::torrent_status::total),
            "total_done",             sol::readonly(&lt::torrent_status::total_done),
            "total_download",         sol::readonly(&lt::torrent_status::total_download),
            "total_failed_bytes",     sol::readonly(&lt::torrent_status::total_failed_bytes),
            "total_payload_download", sol::readonly(&lt::torrent_status::total_payload_download),
            "total_payload_upload",   sol::readonly(&lt::torrent_status::total_payload_upload),
            "total_redundant_bytes",  sol::readonly(&lt::torrent_status::total_redundant_bytes),
            "total_wanted",           sol::readonly(&lt::torrent_status::total_wanted),
            "total_wanted_done",      sol::readonly(&lt::torrent_status::total_wanted_done),
            "total_upload",           sol::readonly(&lt::torrent_status::total_upload),
            "up_bandwidth_queue",     sol::readonly(&lt::torrent_status::up_bandwidth_queue),
            "upload_payload_rate",    sol::readonly(&lt::torrent_status::upload_payload_rate),
            "upload_rate",            sol::readonly(&lt::torrent_status::upload_rate),
            "upload_limit",           sol::readonly(&lt::torrent_status::upload_limit),
            "uploads_limit",          sol::readonly(&lt::torrent_status::uploads_limit)
            // verified_pieces
            );

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
        lua.registry()["db"]          = porla::Lua::Registry::Sqlite3{.db = load_options.db};
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

        porla["unix_time"] = []()
        {
            const auto time_since_epoch = std::chrono::system_clock::now().time_since_epoch();
            return std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
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
                metrics_tbl[m.name] = lua.create_table();
                metrics_tbl[m.name]["type"] = m.type == lt::metric_type_t::counter
                    ? "counter"
                    : "gauge";
            }

            return metrics_tbl;
        };

        porla["sessions"] = [](sol::this_state s)
        {
            sol::state_view lua{s};

            auto  db       = lua.registry()["db"].get<porla::Lua::Registry::Sqlite3>().db;
            auto& sessions = lua.registry()["sessions"].get<porla::Lua::Registry::Sessions>().sessions;

            return std::make_shared<SessionsHandle>(db, sessions);
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

        // Executed on the main state - it may not yield. It should only build and return
        // the plugin table; anything that sleeps or does IO belongs in init().
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
            state->SpawnCoroutine("init", *init);
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