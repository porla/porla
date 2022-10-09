#include "addtorrentparams.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>

#include "../statement.hpp"

using porla::Data::Statement;
using porla::Data::Models::AddTorrentParams;

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

int AddTorrentParams::Count(sqlite3 *db)
{
    int count = 0;

    sqlite3_exec(
        db,
        "SELECT COUNT(*) FROM addtorrentparams",
        [](void* user,int,char** cols,char**)
        {
            int* c = reinterpret_cast<int*>(user);
            *c = std::stoi(cols[0]);

            return SQLITE_OK;
        },
        &count,
        nullptr);

    return count;
}

void AddTorrentParams::ForEach(sqlite3 *db, const std::function<void(libtorrent::add_torrent_params&)>& cb)
{
    auto stmt = Statement::Prepare(db, "SELECT resume_data_buf FROM addtorrentparams\n"
                                       "ORDER BY queue_position ASC");
    stmt.Step(
        [&cb](const Statement::IRow& row)
        {
            auto buf = row.GetBuffer(0);

            libtorrent::error_code ec;
            auto params = lt::read_resume_data(buf, ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read resume data from buffer: " << ec;
                return SQLITE_OK;
            }

            cb(params);

            return SQLITE_OK;
        });
}

void AddTorrentParams::Insert(sqlite3 *db, const libtorrent::add_torrent_params &params, int pos)
{
    std::vector<char> buf = lt::write_resume_data_buf(params);

    auto stmt = Statement::Prepare(db, "INSERT INTO addtorrentparams\n"
                                       "    (info_hash_v1, info_hash_v2, queue_position, resume_data_buf)\n"
                                       "VALUES ($1, $2, $3, $4);");
    stmt
        .Bind(1, params.info_hashes.has_v1()
            ? std::optional(ToString(params.info_hashes.v1))
            : std::nullopt)
        .Bind(2, params.info_hashes.has_v2()
            ? std::optional(ToString(params.info_hashes.v2))
            : std::nullopt)
        .Bind(3, pos)
        .Bind(4, buf)
        .Execute();
}

void AddTorrentParams::Update(sqlite3 *db, const libtorrent::add_torrent_params &params, int pos)
{
    std::vector<char> buf = lt::write_resume_data_buf(params);

    auto stmt = Statement::Prepare(db, "UPDATE addtorrentparams SET resume_data_buf = $1, queue_position = $2\n"
                                       "WHERE (info_hash_v1 = $3 AND info_hash_v2 IS NULL)\n"
                                       "   OR (info_hash_v1 IS NULL AND info_hash_v2 = $4)\n"
                                       "   OR (info_hash_v1 = $3 AND info_hash_v2 = $4);");
    stmt
        .Bind(1, buf)
        .Bind(2, pos)
        .Bind(3, params.info_hashes.has_v1()
                 ? std::optional(ToString(params.info_hashes.v1))
                 : std::nullopt)
        .Bind(4, params.info_hashes.has_v2()
                 ? std::optional(ToString(params.info_hashes.v2))
                 : std::nullopt)
        .Execute();
}
