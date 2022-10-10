#include "session.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <sqlite3ext.h>

#include "data/models/addtorrentparams.hpp"
#include "data/models/sessionparams.hpp"

namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
using porla::Data::Models::SessionParams;
using porla::Session;

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

    if(sqlite3_declare_vtab(db, "create table torrents (info_hash,name)") != SQLITE_OK)
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

    switch (i)
    {
    case 0:
    {
        sqlite3_result_pointer(ctx, &cursor->current->second.info_hashes, "ih", nullptr);
        break;
    }
    case 1:
    {
        sqlite3_result_text(ctx, cursor->current->second.name.c_str(), -1, SQLITE_TRANSIENT);
        break;
    }
    }

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
    , m_timer(io)
    , m_tdb(nullptr)
{
    m_session = std::make_unique<lt::session>();
    m_session->set_alert_notify(
        [this]()
        {
            boost::asio::post(m_io, [this] { ReadAlerts(); });
        });

    boost::system::error_code ec;
    m_timer.expires_from_now(boost::posix_time::seconds(1), ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to set timer expiry: " << ec;
    }
    else
    {
        m_timer.async_wait(
            [this](auto &&PH1)
            {
                PostUpdates(std::forward<decltype(PH1)>(PH1));
            });
    }

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
    m_timer.cancel();

    SessionParams::Insert(
        m_db,
        m_session->session_state());

    m_session->pause();

    // Save each torrents resume data
    int outstanding = 0;
    int paused = 0;
    int failed = 0;

    auto temp = m_session->get_torrent_status([](lt::torrent_status const&) { return true; });

    for (lt::torrent_status& st : temp)
    {
        if (!st.handle.is_valid()
            || !st.has_metadata
            || !st.need_save_resume)
        {
            continue;
        }

        st.handle.save_resume_data(
            lt::torrent_handle::flush_disk_cache
            | lt::torrent_handle::save_info_dict
            | lt::torrent_handle::only_if_modified);

        outstanding++;
    }

    BOOST_LOG_TRIVIAL(info) << "Saving data for " << outstanding << " torrent(s)";

    while (outstanding > 0)
    {
        lt::alert const* tmp = m_session->wait_for_alert(lt::seconds(10));
        if (tmp == nullptr) { continue; }

        std::vector<lt::alert*> alerts;
        m_session->pop_alerts(&alerts);

        for (lt::alert* a : alerts)
        {
            auto* tp = lt::alert_cast<lt::torrent_paused_alert>(a);

            if (tp)
            {
                paused++;
                continue;
            }

            if (auto fail = lt::alert_cast<lt::save_resume_data_failed_alert>(a))
            {
                failed++;
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

            AddTorrentParams::Update(
                m_db,
                rd->params,
                static_cast<int>(rd->handle.status().queue_position));
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

            if (current % 1000 == 0 && current != count)
            {
                BOOST_LOG_TRIVIAL(info) << current << " torrents (of " << count << ") added";
            }

            lt::torrent_handle th = m_session->add_torrent(params);
            m_torrents.insert({ th.info_hashes(), th.status() });
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
    }
}

void Session::AddTorrent(lt::add_torrent_params const& p)
{
    lt::error_code ec;
    lt::torrent_handle th = m_session->add_torrent(p, ec);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to add torrent: " << ec;
        return;
    }

    m_torrents.insert({ th.info_hashes(), th.status() });

    AddTorrentParams::Insert(
        m_db,
        p,
        static_cast<int>(th.status().queue_position));
}

void Session::ForEach(const std::function<void(const libtorrent::torrent_status &)> &cb)
{
    for (auto const& [hash,status] : m_torrents)
    {
        cb(status);
    }
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

void Session::Remove(const lt::info_hash_t& hash)
{
    lt::torrent_status status = m_torrents.at(hash);
    m_session->remove_torrent(status.handle);
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

            AddTorrentParams::Update(
                m_db,
                srda->params,
                static_cast<int>(status.queue_position));

            BOOST_LOG_TRIVIAL(info) << "Resume data saved for " << status.name;

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
            m_torrentPaused(status);

            break;
        }
        }
    }
}

void Session::PostUpdates(boost::system::error_code ec)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "Error in timer: " << ec;
        return;
    }

    m_session->post_dht_stats();
    m_session->post_session_stats();
    m_session->post_torrent_updates();

    m_timer.expires_from_now(boost::posix_time::seconds(1), ec);
    m_timer.async_wait([this](auto && PH1) { PostUpdates(std::forward<decltype(PH1)>(PH1)); });
}
