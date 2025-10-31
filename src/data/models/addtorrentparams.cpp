#include "addtorrentparams.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <nlohmann/json.hpp>

#include "../statement.hpp"
#include "../../json/all.hpp"
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

int AddTorrentParams::Count(sqlite3 *db, const std::string& session)
{
    int count = 0;

    auto stmt = Statement::Prepare(db, "SELECT COUNT(*) AS count FROM addtorrentparams WHERE session_id = $session_id");
    stmt.Bind("$session_id", session);
    stmt.Step(
        [&](const Statement::IRow& row)
        {
            count = row.GetInt32("count");
            return SQLITE_OK;
        });

    return count;
}

void AddTorrentParams::ForEach(sqlite3 *db, const std::string& session, const std::function<void(lt::add_torrent_params&)>& cb)
{
    auto stmt = Statement::Prepare(db, "SELECT client_data,name,resume_data_buf,save_path FROM addtorrentparams\n"
                                       "WHERE session_id = $session_id\n"
                                       "ORDER BY queue_position ASC");
    stmt.Bind("$session_id", session);
    stmt.Step(
        [&cb](const Statement::IRow& row)
        {
            libtorrent::error_code ec;
            auto atp = lt::read_resume_data(row.GetBuffer("resume_data_buf"), ec);

            if (ec)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to read resume data from buffer: " << ec;
                return SQLITE_OK;
            }

            atp.userdata = lt::client_data_t(new TorrentClientData());
            atp.name = row.GetStdString("name");
            atp.save_path = row.GetStdString("save_path");

            const auto client_data_str = row.GetStdString("client_data");

            if (!client_data_str.empty())
            {
                json client_data_json;

                try
                {
                    client_data_json = json::parse(client_data_str);
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to parse client data JSON: " << e.what();
                    return SQLITE_OK;
                }

                if (client_data_json.contains("category") && client_data_json.at("category").is_string())
                {
                    atp.userdata.get<TorrentClientData>()->category = client_data_json["category"];
                }

                if (client_data_json.contains("metadata"))
                {
                    atp.userdata.get<TorrentClientData>()->metadata = client_data_json["metadata"];
                }

                if (client_data_json.contains("tags"))
                {
                    atp.userdata.get<TorrentClientData>()->tags = client_data_json["tags"];
                }
            }

            cb(atp);

            return SQLITE_OK;
        });
}

void AddTorrentParams::Insert(sqlite3 *db, const std::string& session, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    const std::vector<char> buf = lt::write_resume_data_buf(params.params);
    const std::string client_data_json = json(*params.client_data).dump();

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        INSERT INTO addtorrentparams (
            info_hash_v1,
            info_hash_v2,
            client_data,
            name,
            queue_position,
            resume_data_buf,
            save_path,
            session_id
        )
        VALUES (
            $info_hash_v1,
            $info_hash_v2,
            $client_data,
            $name,
            $queue_position,
            $resume_data_buf,
            $save_path,
            $session_id
        );
        )sql");
    stmt
        .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$client_data", client_data_json)
        .Bind("$name", params.name)
        .Bind("$queue_position", params.queue_position)
        .Bind("$resume_data_buf", buf)
        .Bind("$save_path", params.save_path)
        .Bind("$session_id", session)
        .Execute();
}

void AddTorrentParams::Remove(sqlite3 *db, const std::string& session, const libtorrent::info_hash_t& hash)
{
    auto stmt = Statement::Prepare(
        db,
        R"sql(
        DELETE FROM addtorrentparams
        WHERE ((info_hash_v1 = $info_hash_v1 AND info_hash_v2 IS NULL)
           OR (info_hash_v1 IS NULL AND info_hash_v2 = $info_hash_v2)
           OR (info_hash_v1 = $info_hash_v1 AND info_hash_v2 = $info_hash_v2))
        AND session_id = $session_id;
        )sql");

    stmt
        .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",   session)
        .Execute();
}

void AddTorrentParams::Update(sqlite3 *db, const std::string& session, const libtorrent::info_hash_t& hash, const AddTorrentParams& params)
{
    const std::vector<char> buf = lt::write_resume_data_buf(params.params);
    const std::string client_data_json = json(*params.client_data).dump();

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        UPDATE addtorrentparams
        SET
            client_data     = $client_data,
            name            = $name,
            resume_data_buf = $resume_data_buf,
            queue_position  = $queue_position,
            save_path       = $save_path
        WHERE
            (
                (info_hash_v1 = $info_hash_v1 AND info_hash_v2 IS NULL)
                OR (info_hash_v1 IS NULL AND info_hash_v2 = $info_hash_v2)
                OR (info_hash_v1 = $info_hash_v1 AND info_hash_v2 = $info_hash_v2)
            )
            AND session_id = $session_id;
        )sql");

    stmt
        .Bind("$client_data",     client_data_json)
        .Bind("$name",            params.name)
        .Bind("$resume_data_buf", buf)
        .Bind("$queue_position",  params.queue_position)
        .Bind("$save_path",       params.save_path)
        .Bind("$info_hash_v1",    hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2",    hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",      session)
        .Execute();
}
