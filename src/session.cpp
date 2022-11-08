#include "session.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/session_stats.hpp>
#include <utility>
#include <sqlite3ext.h>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessionparams.hpp"

namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
using porla::Data::Models::SessionParams;
using porla::Session;

#define COL_INFO_HASH_V1
#define COL_INFO_HASH_V2
#define COL_ACTIVE_DURATION
#define COL_ALL_TIME_DOWNLOAD
#define COL_ALL_TIME_UPLOAD
#define COL_FINISHED_DURATION
#define COL_NAME
#define COL_QUEUE_POSITION
#define COL_SEEDING_DURATION

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

static const std::vector<std::tuple<std::string, std::function<void(const lt::torrent_status&, sqlite3_context*)>>> Tbl =
{
    {"info_hash_v1",
        [](const lt::torrent_status& ts, sqlite3_context* ctx)
        {
            if (ts.info_hashes.has_v1())
            {
                std::string hash = ToString(ts.info_hashes.v1);
                sqlite3_result_text(ctx, hash.c_str(), -1, SQLITE_TRANSIENT);
            }
            else
            {
                sqlite3_result_null(ctx);
            }
        }},
    {"info_hash_v2", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
         if (ts.info_hashes.has_v2())
         {
             std::string hash = ToString(ts.info_hashes.v2);
             sqlite3_result_text(ctx, hash.c_str(), -1, SQLITE_TRANSIENT);
         }
         else
         {
             sqlite3_result_null(ctx);
         }
    }},
    {"errc_value", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        ts.errc
            ? sqlite3_result_int(ctx, ts.errc.value())
            : sqlite3_result_null(ctx);
    }},
    {"save_path", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_text(ctx, ts.save_path.c_str(), -1, SQLITE_TRANSIENT);
    }},
    {"name", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_text(ctx, ts.name.c_str(), -1, SQLITE_TRANSIENT);
    }},
    {"next_announce", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, lt::total_seconds(ts.next_announce));
    }},
    {"current_tracker", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_text(ctx, ts.current_tracker.c_str(), -1, SQLITE_TRANSIENT);
    }},
    {"total_download", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_download);
    }},
    {"total_upload", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_upload);
    }},
    {"total_payload_download", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_payload_download);
    }},
    {"total_payload_upload", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_payload_upload);
    }},
    {"total_failed_bytes", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_failed_bytes);
    }},
    {"total_redundant_bytes", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_redundant_bytes);
    }},
    // TODO: pieces
    // TODO: verified_pieces
    {"total_done", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_done);
    }},
    {"total", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total);
    }},
    {"total_wanted_done", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_wanted_done);
    }},
    {"total_wanted", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.total_wanted);
    }},
    {"all_time_upload", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.all_time_upload);
    }},
    {"all_time_download", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.all_time_download);
    }},
    {"added_time", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.added_time);
    }},
    {"completed_time", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.completed_time);
    }},
    {"last_seen_complete", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.last_seen_complete);
    }},
    // TODO: storage_mode
    {"progress", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_double(ctx, ts.progress);
    }},
    {"queue_position", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, static_cast<int>(ts.queue_position));
    }},
    {"download_rate", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.download_rate);
    }},
    {"upload_rate", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.upload_rate);
    }},
    {"download_payload_rate", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.download_payload_rate);
    }},
    {"upload_payload_rate", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.upload_payload_rate);
    }},
    {"num_seeds", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_seeds);
    }},
    {"num_peers", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_peers);
    }},
    {"num_complete", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_complete);
    }},
    {"num_incomplete", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_incomplete);
    }},
    {"list_seeds", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.list_seeds);
    }},
    {"list_peers", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.list_peers);
    }},
    {"connect_candidates", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.connect_candidates);
    }},
    {"num_pieces", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_pieces);
    }},
    // TODO: distributed_copies
    {"block_size", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.block_size);
    }},
    {"num_uploads", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_uploads);
    }},
    {"num_connections", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.num_connections);
    }},
    {"uploads_limit", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.uploads_limit);
    }},
    {"connections_limit", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.connections_limit);
    }},
    {"up_bandwidth_queue", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.up_bandwidth_queue);
    }},
    {"down_bandwidth_queue", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.down_bandwidth_queue);
    }},
    {"seed_rank", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.seed_rank);
    }},
    // TODO: state
    {"need_save_resume", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.need_save_resume ? 1 : 0);
    }},
    {"is_seeding", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.is_seeding ? 1 : 0);
    }},
    {"is_finished", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.is_finished ? 1 : 0);
    }},
    {"has_metadata", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.has_metadata ? 1 : 0);
    }},
    {"has_incoming", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.has_incoming ? 1 : 0);
    }},
    {"moving_storage", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.moving_storage ? 1 : 0);
    }},
    {"announcing_to_trackers", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.announcing_to_trackers ? 1 : 0);
    }},
    {"announcing_to_lsd", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.announcing_to_lsd ? 1 : 0);
    }},
    {"announcing_to_dht", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int(ctx, ts.announcing_to_dht ? 1 : 0);
    }},
    {"last_upload", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, lt::total_seconds(ts.last_upload.time_since_epoch()));
    }},
    {"last_download", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, lt::total_seconds(ts.last_download.time_since_epoch()));
    }},
    {"active_duration", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.active_duration.count());
    }},
    {"finished_duration", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.finished_duration.count());
    }},
    {"seeding_duration", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        sqlite3_result_int64(ctx, ts.seeding_duration.count());
    }},
};

class Session::Timer
{
public:
    explicit Timer(boost::asio::io_context& io, int interval, std::function<void()> cb)
        : m_timer(io)
        , m_interval(interval)
        , m_callback(std::move(cb))
    {
        boost::system::error_code ec;

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec.message(); }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    Timer(Timer&& t) noexcept
        : m_timer(std::move(t.m_timer))
        , m_interval(std::exchange(t.m_interval, 0))
        , m_callback(std::move(t.m_callback))
    {
        boost::system::error_code ec;

        m_timer.cancel(ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to cancel timer: " << ec.message(); }

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec.message(); }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete; // noexcept {}

private:
    void OnExpired(boost::system::error_code ec)
    {
        if (ec == boost::system::errc::operation_canceled)
        {
            return;
        }
        else if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec.message();
            return;
        }

        m_callback();

        m_timer.expires_from_now(boost::posix_time::milliseconds(m_interval), ec);
        if (ec) { BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec; }

        m_timer.async_wait([this](auto &&PH1) { OnExpired(std::forward<decltype(PH1)>(PH1)); });
    }

    boost::asio::deadline_timer m_timer;
    int m_interval;
    std::function<void()> m_callback;
};

struct TorrentVTable
{
    sqlite3_vtab base;
    sqlite3* db;
    std::map<lt::info_hash_t, lt::torrent_status>* torrents;
};

struct TorrentVTableCursor
{
    sqlite3_vtab_cursor base{};
    std::map<lt::info_hash_t, lt::torrent_status>::iterator current;
};

static int vt_destructor(sqlite3_vtab *pVtab)
{
    delete reinterpret_cast<TorrentVTable*>(pVtab);
    return SQLITE_OK;
}

static int vt_create(sqlite3 *db, void* aux, int argc, const char* const* argv, sqlite3_vtab **pp_vt, char **pzErr)
{
    auto vtab = new TorrentVTable();
    vtab->db = db;
    vtab->torrents = static_cast<std::map<lt::info_hash_t, lt::torrent_status>*>(aux);

    std::stringstream spec;
    spec << "CREATE TABLE torrents (\n";

    for (auto const& [col, _] : Tbl)
    {
        spec << col << ",\n";
    }

    spec << "_dummy_);";

    if(sqlite3_declare_vtab(db, spec.str().c_str()) != SQLITE_OK)
    {
        vt_destructor(reinterpret_cast<sqlite3_vtab*>(vtab));
        return SQLITE_ERROR;
    }

    *pp_vt = &vtab->base;

    return SQLITE_OK;
}

static int vt_connect(sqlite3 *db, void *p_aux, int argc, const char* const* argv, sqlite3_vtab **pp_vt, char **pzErr )
{
    return vt_create(db, p_aux, argc, argv, pp_vt, pzErr);
}

static int vt_disconnect(sqlite3_vtab *pVtab)
{
    return vt_destructor(pVtab);
}

static int vt_destroy(sqlite3_vtab *pVtab)
{
    return vt_destructor(pVtab);
}

static int vt_open(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **pp_cursor)
{
    auto vtab = reinterpret_cast<TorrentVTable*>(pVTab);
    auto cursor = new TorrentVTableCursor();
    cursor->current = vtab->torrents->begin();

    *pp_cursor = reinterpret_cast<sqlite3_vtab_cursor*>(cursor);

    return SQLITE_OK;
}

static int vt_close(sqlite3_vtab_cursor *cur)
{
    delete reinterpret_cast<TorrentVTableCursor*>(cur);
    return SQLITE_OK;
}

static int vt_eof(sqlite3_vtab_cursor *cur)
{
    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto vtab = reinterpret_cast<TorrentVTable*>(cur->pVtab);

    return cursor->current == vtab->torrents->cend() ? 1 : 0;
}

static int vt_next(sqlite3_vtab_cursor *cur)
{
    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto vtab = reinterpret_cast<TorrentVTable*>(cur->pVtab);

    if (cursor->current != vtab->torrents->cend())
    {
        cursor->current++;
    }

    return SQLITE_OK;
}

static int vt_column(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i)
{
    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto const& status = cursor->current->second;

    auto const& [_, resolver] = Tbl.at(i);
    resolver(status, ctx);

    return SQLITE_OK;
}

static int vt_rowid(sqlite3_vtab_cursor *cur, sqlite_int64 *p_rowid)
{
    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto vtab = reinterpret_cast<TorrentVTable*>(cur->pVtab);

    *p_rowid = std::distance(vtab->torrents->begin(), cursor->current);

    return SQLITE_OK;
}

static int vt_filter(sqlite3_vtab_cursor *p_vtc, int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
    auto cursor = reinterpret_cast<TorrentVTableCursor*>(p_vtc);
    auto vtab = reinterpret_cast<TorrentVTable*>(p_vtc->pVtab);

    cursor->current = vtab->torrents->begin();

    return SQLITE_OK;
}

static int vt_best_index(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo)
{
    return SQLITE_OK;
}

static sqlite3_module PorlaSqliteModule =
{
    0,              /* iVersion */
    vt_create,      /* xCreate       - create a vtable */
    vt_connect,     /* xConnect      - associate a vtable with a connection */
    vt_best_index,  /* xBestIndex    - best index */
    vt_disconnect,  /* xDisconnect   - disassociate a vtable with a connection */
    vt_destroy,     /* xDestroy      - destroy a vtable */
    vt_open,        /* xOpen         - open a cursor */
    vt_close,       /* xClose        - close a cursor */
    vt_filter,      /* xFilter       - configure scan constraints */
    vt_next,        /* xNext         - advance a cursor */
    vt_eof,         /* xEof          - inidicate end of result set*/
    vt_column,      /* xColumn       - read data */
    vt_rowid,       /* xRowid        - read data */
    nullptr,           /* xUpdate       - write data */
    nullptr,           /* xBegin        - begin transaction */
    nullptr,           /* xSync         - sync transaction */
    nullptr,           /* xCommit       - commit transaction */
    nullptr,           /* xRollback     - rollback transaction */
    nullptr,           /* xFindFunction - function overloading */
    nullptr,           /* xRename       - function overloading */
    nullptr,           /* xSavepoint    - function overloading */
    nullptr,           /* xRelease      - function overloading */
    nullptr            /* xRollbackto   - function overloading */
};

Session::Session(boost::asio::io_context& io, porla::SessionOptions const& options)
    : m_io(io)
    , m_db(options.db)
    , m_stats(lt::session_stats_metrics())
    , m_tdb(nullptr)
{
    m_session = std::make_unique<lt::session>();
    m_session->set_alert_notify(
        [this]()
        {
            boost::asio::post(m_io, [this] { ReadAlerts(); });
        });

    if (options.timer_dht_stats > 0)
        m_timers.emplace_back(m_io, options.timer_dht_stats, [&]() { m_session->post_dht_stats(); });

    if (options.timer_session_stats > 0)
        m_timers.emplace_back(m_io, options.timer_session_stats, [&]() { m_session->post_session_stats(); });

    if (options.timer_torrent_updates > 0)
        m_timers.emplace_back(m_io, options.timer_torrent_updates, [&]() { m_session->post_torrent_updates(); });

    sqlite3_open(":memory:", &m_tdb);
    if (sqlite3_create_module(m_tdb, "porla", &PorlaSqliteModule, &m_torrents) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << sqlite3_errmsg(m_tdb);
    }

    if (sqlite3_exec(m_tdb, "create virtual table torrents using porla", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << sqlite3_errmsg(m_tdb);
    }
}

Session::~Session()
{
    BOOST_LOG_TRIVIAL(info) << "Shutting down session";

    m_session->set_alert_notify([]{});
    m_timers.clear();

    SessionParams::Insert(
        m_db,
        m_session->session_state());

    m_session->pause();

    int chunk_size = 1000;
    int chunks = static_cast<int>(m_torrents.size() / chunk_size) + 1;

    BOOST_LOG_TRIVIAL(info) << "Saving resume data in " << chunks << " chunk(s) - total torrents: " << m_torrents.size();

    auto current = m_torrents.begin();

    for (int i = 0; i < chunks; i++)
    {
        int chunk_items = std::min(
            chunk_size,
            static_cast<int>(std::distance(current, m_torrents.end())));

        int outstanding = 0;

        for (int j = 0; j < chunk_items; j++)
        {
            auto const& ts = current->second;

            if (!ts.handle.is_valid()
                || !ts.has_metadata
                || !ts.need_save_resume)
            {
                std::advance(current, 1);
                continue;
            }

            ts.handle.save_resume_data(
                lt::torrent_handle::flush_disk_cache
                | lt::torrent_handle::save_info_dict
                | lt::torrent_handle::only_if_modified);

            outstanding++;

            std::advance(current, 1);
        }

        BOOST_LOG_TRIVIAL(info) << "Chunk " << i + 1 << " - Saving state for " << outstanding << " torrent(s) (out of " << chunk_items << ")";

        while (outstanding > 0)
        {
            lt::alert const* tmp = m_session->wait_for_alert(lt::seconds(10));
            if (tmp == nullptr) { continue; }

            std::vector<lt::alert*> alerts;
            m_session->pop_alerts(&alerts);

            for (lt::alert* a : alerts)
            {
                if (lt::alert_cast<lt::torrent_paused_alert>(a))
                {
                    continue;
                }

                if (auto fail = lt::alert_cast<lt::save_resume_data_failed_alert>(a))
                {
                    outstanding--;

                    BOOST_LOG_TRIVIAL(error)
                        << "Failed to save resume data for "
                        << fail->torrent_name()
                        << ": " << fail->message();

                    continue;
                }

                auto* rd = lt::alert_cast<lt::save_resume_data_alert>(a);
                if (!rd) { continue; }

                outstanding--;

                AddTorrentParams::Update(m_db, rd->handle.info_hashes(), AddTorrentParams{
                    .name = rd->params.name,
                    .params = rd->params,
                    .queue_position = static_cast<int>(rd->handle.status().queue_position),
                    .save_path = rd->params.save_path
                });
            }
        }
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";
}

void Session::Load()
{
    int count = AddTorrentParams::Count(m_db);
    int current = 0;

    BOOST_LOG_TRIVIAL(info) << "Loading " << count << " torrent(s) from storage";

    AddTorrentParams::ForEach(
        m_db,
        [&](lt::add_torrent_params& params)
        {
            current++;

            lt::torrent_handle th = m_session->add_torrent(params);
            m_torrents.insert({ th.info_hashes(), th.status() });

            if (current % 1000 == 0 && current != count)
            {
                BOOST_LOG_TRIVIAL(info) << current << " torrents (of " << count << ") added";
            }
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
    }
}

lt::info_hash_t Session::AddTorrent(lt::add_torrent_params const& p)
{
    lt::error_code ec;
    lt::torrent_handle th = m_session->add_torrent(p, ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to add torrent: " << ec;
        return {};
    }

    lt::torrent_status ts = th.status();

    AddTorrentParams::Insert(m_db, ts.info_hashes, AddTorrentParams{
        .name = ts.name,
        .params = p,
        .queue_position = static_cast<int>(ts.queue_position),
        .save_path = ts.save_path,
    });

    th.save_resume_data(
        lt::torrent_handle::flush_disk_cache
        | lt::torrent_handle::save_info_dict
        | lt::torrent_handle::only_if_modified);

    m_torrents.insert({ ts.info_hashes, ts });
    m_torrentAdded(ts);

    return ts.info_hashes;
}

void Session::Pause()
{
    m_session->pause();
}

void Session::Query(const std::string_view& query, const std::function<int(sqlite3_stmt*)>& cb)
{
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_tdb, query.data(), -1, &stmt, nullptr);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        cb(stmt);
    }

    sqlite3_finalize(stmt);
}

void Session::Remove(const lt::info_hash_t& hash, bool remove_data)
{
    lt::torrent_status status = m_torrents.at(hash);

    m_session->remove_torrent(status.handle, remove_data ? lt::session::delete_files : lt::remove_flags_t{});
}

void Session::Resume()
{
    m_session->resume();
}

lt::settings_pack Session::Settings()
{
    return m_session->get_settings();
}

const std::map<lt::info_hash_t, lt::torrent_status>& Session::Torrents()
{
    return m_torrents;
}

void Session::ReadAlerts()
{
    std::vector<lt::alert*> alerts;
    m_session->pop_alerts(&alerts);

    for (auto const alert : alerts)
    {
        // BOOST_LOG_TRIVIAL(trace) << alert->message();
        switch (alert->type())
        {
        case lt::dht_stats_alert::alert_type:
        {
            auto dsa = lt::alert_cast<lt::dht_stats_alert>(alert);
            // TODO: emit signal
            break;
        }
        case lt::metadata_received_alert::alert_type:
        {
            auto mra = lt::alert_cast<lt::metadata_received_alert>(alert);

            BOOST_LOG_TRIVIAL(info) << "Metadata received for torrent " << mra->handle.status().name;

            mra->handle.save_resume_data(
                lt::torrent_handle::flush_disk_cache
                | lt::torrent_handle::save_info_dict
                | lt::torrent_handle::only_if_modified);

            break;
        }
        case lt::save_resume_data_alert::alert_type:
        {
            auto srda = lt::alert_cast<lt::save_resume_data_alert>(alert);
            auto const& status = m_torrents.at(srda->handle.info_hashes());

            AddTorrentParams::Update(m_db, status.info_hashes, AddTorrentParams{
                .name = status.name,
                .params = srda->params,
                .queue_position = static_cast<int>(status.queue_position),
                .save_path = status.save_path
            });

            BOOST_LOG_TRIVIAL(info) << "Resume data saved for " << status.name;

            break;
        }
        case lt::session_stats_alert::alert_type:
        {
            auto ssa = lt::alert_cast<lt::session_stats_alert>(alert);
            auto const& counters = ssa->counters();

            std::map<std::string, int64_t> metrics;

            for (auto const& stats : m_stats)
            {
                metrics.insert({ stats.name, counters[stats.value_index] });
            }

            m_sessionStats(metrics);

            break;
        }
        case lt::state_update_alert::alert_type:
        {
            auto sua = lt::alert_cast<lt::state_update_alert>(alert);

            for (auto const& s : sua->status)
            {
                m_torrents.at(s.info_hashes) = s;
            }

            m_stateUpdate(sua->status);

            break;
        }
        case lt::torrent_finished_alert::alert_type:
        {
            auto tfa = lt::alert_cast<lt::torrent_finished_alert>(alert);
            auto const& status = tfa->handle.status();

            m_torrents.at(status.info_hashes) = status;

            if (status.total_download > 0)
            {
                // Only emit this event if we have downloaded any data this session.
                m_torrentFinished(status);
            }

            break;
        }
        case lt::torrent_paused_alert::alert_type:
        {
            auto tpa = lt::alert_cast<lt::torrent_paused_alert>(alert);
            auto const& status = tpa->handle.status();

            m_torrents.at(status.info_hashes) = status;
            m_torrentPaused(status);

            break;
        }
        case lt::torrent_removed_alert::alert_type:
        {
            auto tra = lt::alert_cast<lt::torrent_removed_alert>(alert);

            AddTorrentParams::Remove(m_db, tra->info_hashes);

            m_torrents.erase(tra->info_hashes);
            m_torrentRemoved(tra->info_hashes);

            BOOST_LOG_TRIVIAL(info) << "Torrent " << tra->torrent_name() << " removed";

            break;
        }
        case lt::torrent_resumed_alert::alert_type:
        {
            auto tra = lt::alert_cast<lt::torrent_resumed_alert>(alert);
            auto const& status = tra->handle.status();

            m_torrents.at(status.info_hashes) = status;
            m_torrentResumed(status);

            break;
        }
        }
    }
}
