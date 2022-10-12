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

void AddTorrentParams::ForEach(sqlite3 *db, const std::function<void(lt::add_torrent_params&)>& cb)
{
    auto stmt = Statement::Prepare(db, "SELECT name,resume_data_buf,save_path FROM addtorrentparams\n"
                                       "ORDER BY queue_position ASC");
    stmt.Step(
        [&cb](const Statement::IRow& row)
        {

            libtorrent::error_code ec;
            auto atp = lt::read_resume_data(row.GetBuffer(1), ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read resume data from buffer: " << ec;
                return SQLITE_OK;
            }

            atp.name = row.GetStdString(0);
            atp.save_path = row.GetStdString(2);

            cb(atp);

            return SQLITE_OK;
        });
}

void AddTorrentParams::Insert(sqlite3 *db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    std::vector<char> buf = lt::write_resume_data_buf(params.params);

    auto stmt = Statement::Prepare(db, "INSERT INTO addtorrentparams\n"
                                       "    (info_hash_v1, info_hash_v2, name, queue_position, resume_data_buf, save_path)\n"
                                       "VALUES ($1, $2, $3, $4, $5, $6);");
    stmt
        .Bind(1, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(2, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind(3, std::string_view(params.name))
        .Bind(4, params.queue_position)
        .Bind(5, buf)
        .Bind(6, std::string_view(params.save_path))
        .Execute();
}

void AddTorrentParams::Update(sqlite3 *db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    std::vector<char> buf = lt::write_resume_data_buf(params.params);

    auto stmt = Statement::Prepare(db, "UPDATE addtorrentparams SET name = $1, resume_data_buf = $2, queue_position = $3, save_path = $4\n"
                                       "WHERE (info_hash_v1 = $5 AND info_hash_v2 IS NULL)\n"
                                       "   OR (info_hash_v1 IS NULL AND info_hash_v2 = $6)\n"
                                       "   OR (info_hash_v1 = $5 AND info_hash_v2 = $6);");
    stmt
        .Bind(1, std::string_view(params.name))
        .Bind(2, buf)
        .Bind(3, params.queue_position)
        .Bind(4, std::string_view(params.save_path))
        .Bind(5, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(6, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Execute();
}
