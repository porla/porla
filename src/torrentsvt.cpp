#include "torrentsvt.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/torrent_status.hpp>
#include <sqlite3ext.h>

using porla::TorrentsVTable;

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
    {"state", [](const lt::torrent_status& ts, sqlite3_context* ctx) {
        switch (ts.state)
        {
        case libtorrent::torrent_status::checking_files:
            sqlite3_result_text(ctx, "checking_files", -1, SQLITE_TRANSIENT);
            break;
        case libtorrent::torrent_status::downloading_metadata:
            sqlite3_result_text(ctx, "downloading_metadata", -1, SQLITE_TRANSIENT);
            break;
        case libtorrent::torrent_status::downloading:
            sqlite3_result_text(ctx, "downloading", -1, SQLITE_TRANSIENT);
            break;
        case libtorrent::torrent_status::finished:
            sqlite3_result_text(ctx, "finished", -1, SQLITE_TRANSIENT);
            break;
        case libtorrent::torrent_status::seeding:
            sqlite3_result_text(ctx, "seeding", -1, SQLITE_TRANSIENT);
            break;
        case libtorrent::torrent_status::checking_resume_data:
            sqlite3_result_text(ctx, "checking_resume_data", -1, SQLITE_TRANSIENT);
            break;
        default:
            sqlite3_result_text(ctx, "unknown", -1, SQLITE_TRANSIENT);
            break;
        }
    }},
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

struct TorrentVTable
{
    sqlite3_vtab base;
    sqlite3* db;
    std::map<lt::info_hash_t, lt::torrent_handle>* torrents;
};

struct TorrentVTableCursor
{
    sqlite3_vtab_cursor base{};
    std::map<lt::info_hash_t, lt::torrent_handle>::iterator current;
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
    vtab->torrents = static_cast<std::map<lt::info_hash_t, lt::torrent_handle>*>(aux);

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
    auto const& th = cursor->current->second;

    auto const& [_, resolver] = Tbl.at(i);
    resolver(th.status(), ctx); // TODO: Check torrent status flags

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
    nullptr,        /* xUpdate       - write data */
    nullptr,        /* xBegin        - begin transaction */
    nullptr,        /* xSync         - sync transaction */
    nullptr,        /* xCommit       - commit transaction */
    nullptr,        /* xRollback     - rollback transaction */
    nullptr,        /* xFindFunction - function overloading */
    nullptr,        /* xRename       - function overloading */
    nullptr,        /* xSavepoint    - function overloading */
    nullptr,        /* xRelease      - function overloading */
    nullptr         /* xRollbackto   - function overloading */
};

int TorrentsVTable::Install(sqlite3* db, std::map<lt::info_hash_t, lt::torrent_handle>& torrents)
{
    int res = sqlite3_create_module(db, "porla", &PorlaSqliteModule, &torrents);

    if (res != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to create 'porla' SQLite module: " << sqlite3_errmsg(db);
        return res;
    }

    res = sqlite3_exec(db, "create virtual table torrents using porla", nullptr, nullptr, nullptr);

    if (res != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to create virtual 'torrents' table: " << sqlite3_errmsg(db);
        return res;
    }

    return SQLITE_OK;
}
