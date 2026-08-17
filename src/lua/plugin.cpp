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

    void writeHeader(std::string_view key, std::string_view val)
    {
        if (m_aborted) { return; }
        m_res->writeHeader(key, val);
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
    std::map<std::size_t, std::shared_ptr<CronSchedule>>      m_cron_schedules;

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

    sol::object AddCronSchedule(const std::string& expr, sol::protected_function callback)
    {
        if (!callback.valid())
        {
            BOOST_LOG_TRIVIAL(warning) << Name() << ": cron(...) called without a function";
            return sol::nil;
        }

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
                    if (!self) return;

                    callback();
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

        lua.new_usertype<porla::CurlMulti>("CurlMulti", sol::no_constructor);

        lua.new_usertype<uWS::HttpRequest>(
            "http_server.Request",
            sol::no_constructor);

        lua.new_usertype<HttpResponseHandle>(
            "http_server.Response",
            sol::no_constructor,
            "finish", sol::overload(
                [](HttpResponseHandle& h) { h.end(); },
                [](HttpResponseHandle& h, std::string_view data) { h.end(data); }
            ),
            "write", &HttpResponseHandle::write,
            "writeHeader", &HttpResponseHandle::writeHeader,
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

        lua.new_usertype<lt::announce_endpoint>(
            "lt.announce_endpoint",
            sol::no_constructor,
            "enabled",     &lt::announce_endpoint::enabled
            // "info_hashes", &lt::announce_endpoint::info_hashes
            // local_endpoint
            );

        lua.new_usertype<lt::announce_entry>(
            "lt.announce_entry",
            sol::no_constructor,
            "endpoints",  &lt::announce_entry::endpoints,
            "fail_limit", &lt::announce_entry::fail_limit,
            // "source",     &lt::announce_entry::source,
            "tier",       &lt::announce_entry::tier,
            "trackerid",  &lt::announce_entry::trackerid,
            "url",        &lt::announce_entry::url,
            "verified",   sol::readonly_property([](const lt::announce_entry& ae) { return ae.verified; }));

        lua.new_usertype<lt::announce_infohash>(
            "lt.announce_infohash",
            sol::no_constructor,
            "complete_sent",     sol::readonly_property([](const lt::announce_infohash& ai) { return ai.complete_sent; }),
            "fails",             sol::readonly_property([](const lt::announce_infohash& ai) { return ai.fails; }),
            // last_error
            "message",           &lt::announce_infohash::message,
            "min_announce",      sol::readonly_property([](const lt::announce_infohash& ai) { return ai.min_announce.time_since_epoch().count(); }),
            "next_announce",     sol::readonly_property([](const lt::announce_infohash& ai) { return ai.next_announce.time_since_epoch().count(); }),
            "scrape_complete",   &lt::announce_infohash::scrape_complete,
            "scrape_downloaded", &lt::announce_infohash::scrape_downloaded,
            "scrape_incomplete", &lt::announce_infohash::scrape_incomplete,
            "start_sent",        sol::readonly_property([](const lt::announce_infohash& ai) { return ai.start_sent; }),
            "updating",          sol::readonly_property([](const lt::announce_infohash& ai) { return ai.updating; }));

        lua.new_usertype<lt::open_file_state>(
            "lt.open_file_state",
            sol::no_constructor,
            "file_index", &lt::open_file_state::file_index,
            "open_mode",  &lt::open_file_state::open_mode,
            "last_use",   sol::readonly_property([](const lt::open_file_state& ofs) { return ofs.last_use.time_since_epoch().count(); }));

        lua.new_usertype<lt::peer_info>(
            "lt.peer_info",
            sol::no_constructor,
            "busy_requests",            sol::readonly(&lt::peer_info::busy_requests),
            "client",                   sol::readonly(&lt::peer_info::client),
            // connection_type
            "down_speed",               sol::readonly(&lt::peer_info::down_speed),
            "download_queue_length",    sol::readonly(&lt::peer_info::download_queue_length),
            "download_rate_peak",       sol::readonly(&lt::peer_info::download_rate_peak),
            // download_queue_time
            "downloading_block_index",  sol::readonly(&lt::peer_info::downloading_block_index),
            // downloading_piece_index
            "downloading_progress",     sol::readonly(&lt::peer_info::downloading_progress),
            "downloading_total",        sol::readonly(&lt::peer_info::downloading_total),
            "failcount",                sol::readonly(&lt::peer_info::failcount),
            // flags
            // last_active
            // last_request
            // local_endpoint
            "num_hashfails",            sol::readonly(&lt::peer_info::num_hashfails),
            "num_pieces",               sol::readonly(&lt::peer_info::num_hashfails),
            "payload_down_speed",       sol::readonly(&lt::peer_info::payload_down_speed),
            "payload_up_speed",         sol::readonly(&lt::peer_info::payload_up_speed),
            "pending_disk_bytes",       sol::readonly(&lt::peer_info::pending_disk_bytes),
            "pending_disk_read_bytes",  sol::readonly(&lt::peer_info::pending_disk_read_bytes),
            // pid
            // pieces
            "progress",                 sol::readonly(&lt::peer_info::progress),
            "queue_bytes",              sol::readonly(&lt::peer_info::queue_bytes),
            // read_state
            "receive_buffer_size",      sol::readonly(&lt::peer_info::receive_buffer_size),
            "receive_buffer_watermark", sol::readonly(&lt::peer_info::receive_buffer_watermark),
            "receive_quota",            sol::readonly(&lt::peer_info::receive_quota),
            // remote_endpoint
            "request_timeout",          sol::readonly(&lt::peer_info::request_timeout),
            "requests_in_buffer",       sol::readonly(&lt::peer_info::requests_in_buffer),
            "rtt",                      sol::readonly(&lt::peer_info::rtt),
            // source
            "send_buffer_size",         sol::readonly(&lt::peer_info::send_buffer_size),
            "send_quota",               sol::readonly(&lt::peer_info::send_quota),
            "target_dl_queue_length",   sol::readonly(&lt::peer_info::target_dl_queue_length),
            "timed_out_requests",       sol::readonly(&lt::peer_info::timed_out_requests),
            "total_download",           sol::readonly(&lt::peer_info::total_download),
            "total_upload",             sol::readonly(&lt::peer_info::total_upload),
            "up_speed",                 sol::readonly(&lt::peer_info::up_speed),
            "upload_queue_length",      sol::readonly(&lt::peer_info::upload_queue_length),
            "upload_rate_peak",         sol::readonly(&lt::peer_info::upload_rate_peak),
            "used_send_buffer",         sol::readonly(&lt::peer_info::used_send_buffer),
            "used_receive_buffer",      sol::readonly(&lt::peer_info::used_receive_buffer)
            // write_state
        );

        lua.new_usertype<lt::torrent_handle>(
            "lt.torrent_handle",
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
            "get_peer_info",              [](const lt::torrent_handle& th)
                                          {
                                              std::vector<lt::peer_info> peers;
                                              th.get_peer_info(peers);
                                              return peers;
                                          },
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
            "post_download_queue",        &lt::torrent_handle::post_download_queue,
            "post_file_priorities",       &lt::torrent_handle::post_file_priorities,
            "post_file_progress",         &lt::torrent_handle::post_file_progress,
            "post_file_status",           &lt::torrent_handle::post_file_status,
            "post_peer_info",             &lt::torrent_handle::post_peer_info,
            "post_piece_availability",    &lt::torrent_handle::post_piece_availability,
            "post_status",                &lt::torrent_handle::post_status,
            "post_trackers",              &lt::torrent_handle::post_trackers,
            // prioritize_files
            // prioritize_pieces
            "queue_position",             &lt::torrent_handle::queue_position,
            "queue_position_bottom",      &lt::torrent_handle::queue_position_bottom,
            "queue_position_down",        &lt::torrent_handle::queue_position_down,
            "queue_position_set",         &lt::torrent_handle::queue_position_set,
            "queue_position_top",         &lt::torrent_handle::queue_position_top,
            "queue_position_up",          &lt::torrent_handle::queue_position_up,
            "read_piece",                 [](const lt::torrent_handle& th, int piece_index)
                                          { th.read_piece(lt::piece_index_t{piece_index}); },
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
            "seeding_duration",       sol::property([](const lt::torrent_status& ts) { return ts.seeding_duration.count(); }),
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
                if (!self) return;

                auto response = std::make_shared<HttpResponseHandle>(res);
                response->Setup();

                callback(req, response);
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
            (*init)();
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
