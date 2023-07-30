#include "addtorrentparams.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <nlohmann/json.hpp>

#include "../statement.hpp"
#include "../../json/torrentclientdata.hpp"
#include "../../torrentclientdata.hpp"

using json = nlohmann::json;
using porla::Data::Statement;
using porla::Data::Models::AddTorrentParams;

template<typename T>
static std::string ToString(const T &hash)
{
    std::stringstream ss;
    ss << hash;
    return ss.str();
}

int AddTorrentParams::Count(sqlite3 *db, const std::string_view& session)
{
    int count = 0;

    auto stmt = Statement::Prepare(db, "SELECT COUNT(*) FROM addtorrentparams WHERE session_id = $1");
    stmt.Bind(1, session);
    stmt.Step(
        [&](const Statement::IRow& row)
        {
            count = row.GetInt32(0);
            return SQLITE_OK;
        });

    return count;
}

void AddTorrentParams::ForEach(sqlite3 *db, const std::function<void(lt::add_torrent_params&)>& cb)
{
    auto stmt = Statement::Prepare(db, "SELECT client_data,name,resume_data_buf,save_path FROM addtorrentparams\n"
                                       "ORDER BY queue_position ASC");
    stmt.Step(
        [&cb](const Statement::IRow& row)
        {

            libtorrent::error_code ec;
            auto atp = lt::read_resume_data(row.GetBuffer(2), ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read resume data from buffer: " << ec;
                return SQLITE_OK;
            }

            atp.userdata = lt::client_data_t(new TorrentClientData());
            atp.name = row.GetStdString(1);
            atp.save_path = row.GetStdString(3);

            const auto client_data_str = row.GetStdString(0);

            if (!client_data_str.empty())
            {
                json::parse(client_data_str).get_to(
                    *atp.userdata.get<TorrentClientData>());
            }

            cb(atp);

            return SQLITE_OK;
        });
}

void AddTorrentParams::Insert(sqlite3 *db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    std::vector<char> buf = lt::write_resume_data_buf(params.params);

    const std::string client_data_json = json(*params.client_data).dump();

    auto stmt = Statement::Prepare(db, "INSERT INTO addtorrentparams\n"
                                       "    (info_hash_v1, info_hash_v2, client_data, name, queue_position, resume_data_buf, save_path)\n"
                                       "VALUES ($1, $2, $3, $4, $5, $6, $7);");
    stmt
        .Bind(1, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(2, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind(3, std::string_view(client_data_json))
        .Bind(4, std::string_view(params.name))
        .Bind(5, params.queue_position)
        .Bind(6, buf)
        .Bind(7, std::string_view(params.save_path))
        .Execute();
}

void AddTorrentParams::Remove(sqlite3 *db, const libtorrent::info_hash_t& hash)
{
    auto stmt = Statement::Prepare(
        db,
        "DELETE FROM addtorrentparams\n"
        "WHERE (info_hash_v1 = $1 AND info_hash_v2 IS NULL)\n"
        "   OR (info_hash_v1 IS NULL AND info_hash_v2 = $2)\n"
        "   OR (info_hash_v1 = $1 AND info_hash_v2 = $2);");

    stmt
        .Bind(1, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(2, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Execute();
}

void AddTorrentParams::Update(sqlite3 *db, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    std::vector<char> buf = lt::write_resume_data_buf(params.params);

    const std::string client_data_json = json(*params.client_data).dump();

    auto stmt = Statement::Prepare(db, "UPDATE addtorrentparams SET client_data = $1, name = $2, resume_data_buf = $3, queue_position = $4, save_path = $5\n"
                                       "WHERE (info_hash_v1 = $6 AND info_hash_v2 IS NULL)\n"
                                       "   OR (info_hash_v1 IS NULL AND info_hash_v2 = $7)\n"
                                       "   OR (info_hash_v1 = $6 AND info_hash_v2 = $7);");
    stmt
        .Bind(1, std::string_view(client_data_json))
        .Bind(2, std::string_view(params.name))
        .Bind(3, buf)
        .Bind(4, params.queue_position)
        .Bind(5, std::string_view(params.save_path))
        .Bind(6, hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind(7, hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Execute();
}
