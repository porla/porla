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
using porla::TorrentClientData;

namespace
{
    static std::optional<lt::add_torrent_params> ReadParams(const Statement::IRow& row)
    {
        auto client_data = std::make_unique<TorrentClientData>();

        const auto params_buffer = row.GetBuffer("params");
        const auto userdata_buffer = row.GetStdString("userdata");

        if (!userdata_buffer.empty())
        {
            json client_data_json;

            try
            {
                client_data_json = json::parse(userdata_buffer);
            }
            catch (const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to parse client data JSON: " << e.what();
                client_data_json = json::object();
            }

            if (client_data_json.contains("category") && client_data_json.at("category").is_string())
            {
                client_data->category = client_data_json["category"];
            }

            if (client_data_json.contains("metadata") && client_data_json.at("metadata").is_object())
            {
                client_data->metadata = client_data_json["metadata"];
            }

            if (client_data_json.contains("tags") && client_data_json.at("tags").is_array())
            {
                client_data->tags = client_data_json["tags"];
            }
        }

        lt::error_code ec;
        lt::add_torrent_params params = lt::read_resume_data(params_buffer, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read resume data from buffer: " << ec;
            return std::nullopt;
        }

        params.userdata = lt::client_data_t(client_data.release());

        return params;
    }

    template<typename T>
    static std::string ToString(const T &hash)
    {
        std::stringstream ss;
        ss << hash;
        return ss.str();
    }

    std::string SerializeClientData(const TorrentClientData& client_data)
    {
        const std::map<std::string, json> userdata = {
            {"category", client_data.category.has_value() ? json(client_data.category.value()) : json()},
            {"metadata", client_data.metadata},
            {"tags",     client_data.tags}
        };

        return json(userdata).dump();
    }
}

int AddTorrentParams::Count(sqlite3 *db, const int session_id)
{
    int count = 0;

    auto stmt = Statement::Prepare(db, "SELECT COUNT(*) AS count FROM addtorrentparams WHERE session_id = $session_id");
    stmt.Bind("$session_id", session_id);
    stmt.Step(
        [&](const Statement::IRow& row)
        {
            count = row.GetInt32("count");
            return SQLITE_OK;
        });

    return count;
}

void AddTorrentParams::Insert(sqlite3 *db, const int session_id, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData& client_data, const int queue_pos)
{
    const std::map<std::string, json> userdata = {
        {"category", client_data.category ? json(client_data.category.value()) : json()},
        {"metadata", client_data.metadata},
        {"tags",     client_data.tags}
    };

    const std::vector<char> buf = lt::write_resume_data_buf(params);
    const std::string userdata_str = json(userdata).dump();

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        INSERT INTO addtorrentparams (
            info_hash_v1,
            info_hash_v2,
            session_id,
            queue_position,
            params,
            userdata
        )
        VALUES (
            $info_hash_v1,
            $info_hash_v2,
            $session_id,
            $queue_position,
            $params,
            $userdata
        );
        )sql");
    stmt
        .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id", session_id)
        .Bind("$queue_position", queue_pos)
        .Bind("$params", buf)
        .Bind("$userdata", userdata_str)
        .Execute();
}

bool AddTorrentParams::Next(
    sqlite3* db,
    const int session_id,
    Cursor& cursor,
    const int max,
    const std::function<void(lt::add_torrent_params&)>& cb)
{
    const auto Select = R"sql(
    SELECT
        id,
        queue_position,
        params,
        userdata
    FROM
        addtorrentparams
    WHERE
        session_id = $session_id
        AND queue_position <= $max_position
        AND queue_position >= $position
        AND (queue_position > $position OR id > $id)
    ORDER BY
        queue_position ASC, id ASC
    LIMIT
        $max
    )sql";

    auto stmt = Statement::Prepare(db, Select);
    stmt.Bind("$session_id", session_id);
    stmt.Bind("$max_position", cursor.unqueued
        ? static_cast<std::int64_t>(-1)
        : std::numeric_limits<std::int64_t>::max());
    stmt.Bind("$position", cursor.position);
    stmt.Bind("$id", cursor.id);
    stmt.Bind("$max", max);

    int rows = 0;

    stmt.Step([&](const auto& row)
    {
        rows++;

        cursor.position = row.GetInt64("queue_position");
        cursor.id       = row.GetInt64("id");

        if (auto params = ReadParams(row))
        {
            cb(params.value());
        }

        return SQLITE_OK;
    });

    if (rows > 0)
    {
        return true;
    }

    if (cursor.unqueued)
    {
        return false;
    }

    cursor.unqueued = true;
    cursor.position = std::numeric_limits<std::int64_t>::min();
    cursor.id       = -1;

    return true;
}

void AddTorrentParams::Remove(sqlite3 *db, const int session_id, const lt::info_hash_t& hash)
{
    auto stmt = Statement::Prepare(
        db,
        R"sql(
        DELETE FROM addtorrentparams
        WHERE
            (session_id = $session_id AND info_hash_v1 = $info_hash_v1
                AND (info_hash_v2 IS NULL OR info_hash_v2 = $info_hash_v2))
            OR
            (session_id = $session_id AND info_hash_v2 = $info_hash_v2
                AND (info_hash_v1 IS NULL OR info_hash_v1 = $info_hash_v1))
        )sql");

    stmt
        .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",   session_id)
        .Execute();
}

void AddTorrentParams::Update(sqlite3 *db, const int session_id, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData* client_data, const int queue_pos)
{
    const std::optional<std::string> userdata = client_data != nullptr
        ? std::optional(SerializeClientData(*client_data))
        : std::nullopt;

    const std::vector<char> buf = lt::write_resume_data_buf(params);

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        UPDATE addtorrentparams
        SET
            queue_position = $queue_position,
            params         = $params,
            userdata       = COALESCE($userdata, userdata)
        WHERE
            (session_id = $session_id AND info_hash_v1 = $info_hash_v1
                AND (info_hash_v2 IS NULL OR info_hash_v2 = $info_hash_v2))
            OR
            (session_id = $session_id AND info_hash_v2 = $info_hash_v2
                AND (info_hash_v1 IS NULL OR info_hash_v1 = $info_hash_v1))
        )sql");

    stmt
        .Bind("$queue_position", queue_pos)
        .Bind("$params",         buf)
        .Bind("$info_hash_v1",   hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2",   hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",     session_id)
        .Bind("$userdata",       userdata)
        .Execute();
}

void AddTorrentParams::UpdateClientData(sqlite3 *db, const int session_id, const lt::info_hash_t& hash, const TorrentClientData& client_data)
{
    const std::string userdata_str = SerializeClientData(client_data);

    auto stmt = Statement::Prepare(
        db,
        R"sql(
        UPDATE addtorrentparams
        SET
            userdata       = $userdata
        WHERE
            (session_id = $session_id AND info_hash_v1 = $info_hash_v1
                AND (info_hash_v2 IS NULL OR info_hash_v2 = $info_hash_v2))
            OR
            (session_id = $session_id AND info_hash_v2 = $info_hash_v2
                AND (info_hash_v1 IS NULL OR info_hash_v1 = $info_hash_v1))
        )sql");

    stmt
        .Bind("$userdata",       userdata_str)
        .Bind("$info_hash_v1",   hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2",   hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",     session_id)
        .Execute();
}
