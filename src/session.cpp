#include "session.hpp"

#include <ranges>

#include <boost/log/trivial.hpp>
#include <libtorrent/alert_types.hpp>
#include <sqlite3ext.h>

#include "data/models/addtorrentparams.hpp"

namespace lt = libtorrent;

using porla::Data::Models::AddTorrentParams;
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
    BOOST_LOG_TRIVIAL(info) << "Creating VTable";

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
    BOOST_LOG_TRIVIAL(info) << "vt_open";

    auto vtab = reinterpret_cast<TorrentVTable*>(pVTab);
    auto cursor = new TorrentVTableCursor();
    cursor->current = vtab->torrents->begin();

    *pp_cursor = reinterpret_cast<sqlite3_vtab_cursor*>(cursor);

    return SQLITE_OK;
}

static int vt_close(sqlite3_vtab_cursor *cur)
{
    BOOST_LOG_TRIVIAL(info) << "vt_close";

    delete reinterpret_cast<TorrentVTableCursor*>(cur);
    return SQLITE_OK;
}

static int vt_eof(sqlite3_vtab_cursor *cur)
{
    BOOST_LOG_TRIVIAL(info) << "vt_eof";

    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto vtab = reinterpret_cast<TorrentVTable*>(cur->pVtab);

    bool eof = cursor->current == vtab->torrents->cend();

    BOOST_LOG_TRIVIAL(info) << "is eof: " << eof;

    return eof ? 1 : 0;
}

static int vt_next(sqlite3_vtab_cursor *cur)
{
    BOOST_LOG_TRIVIAL(info) << "vt_next";

    auto cursor = reinterpret_cast<TorrentVTableCursor*>(cur);
    auto vtab = reinterpret_cast<TorrentVTable*>(cur->pVtab);

    BOOST_LOG_TRIVIAL(info) << (cursor->current == vtab->torrents->cend());

    if (cursor->current != vtab->torrents->cend())
    {
        cursor->current++;
    }

    return SQLITE_OK;
}

static int vt_column(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, int i)
{
    BOOST_LOG_TRIVIAL(info) << "vt_column";

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
    BOOST_LOG_TRIVIAL(info) << "vt_rowid";

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

/* Pretty involved. We don't implement in this example. */
static int vt_best_index(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo)
{
    BOOST_LOG_TRIVIAL(info) << "vt_best_index";

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

struct AddParams
{
    uint32_t current = 0;
    uint32_t total = 0;
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

            auto ap = new AddParams();
            ap->current = current;
            ap->total = count;

            params.userdata = lt::client_data_t(ap);

            m_session->async_add_torrent(params);
        });

    if (count > 0)
    {
        BOOST_LOG_TRIVIAL(info) << "Added " << current << " (of " << count << ") torrent(s) to session";
    }
}

void Session::AddTorrent(lt::add_torrent_params const& p)
{
    m_session->async_add_torrent(p);
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
        case lt::add_torrent_alert::alert_type:
        {
            auto ata = lt::alert_cast<lt::add_torrent_alert>(alert);

            if (ata->error)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to add torrent: " << ata->error;
                continue;
            }

            lt::torrent_status ts = ata->handle.status();
            m_torrents.insert({ ts.info_hashes, ts });

            if (auto extra = ata->params.userdata.get<AddParams>())
            {
                if (extra->current % 1000 == 0
                    && extra->current != extra->total)
                {
                    BOOST_LOG_TRIVIAL(info) << extra->current << " torrents (of " << extra->total << ") added";
                }
                else if (extra->current == extra->total)
                {
                    BOOST_LOG_TRIVIAL(info) << "All torrents added";
                }

                delete extra;
            }
            else
            {
                AddTorrentParams::Insert(m_db, ata->params, static_cast<int>(ts.queue_position));
                BOOST_LOG_TRIVIAL(info) << "Torrent " << ts.name << " added";
            }

            if (ts.need_save_resume)
            {
                ts.handle.save_resume_data(
                    lt::torrent_handle::flush_disk_cache
                    | lt::torrent_handle::save_info_dict
                    | lt::torrent_handle::only_if_modified);
            }

            break;
        }
        case lt::save_resume_data_alert::alert_type:
        {
            auto srda = lt::alert_cast<lt::save_resume_data_alert>(alert);
            AddTorrentParams::Update(m_db, srda->params);
            BOOST_LOG_TRIVIAL(info) << "Resume data saved for " << srda->params.name;
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
