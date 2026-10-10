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
    std::shared_ptr<lt::torrent_info> ReadInfo(const std::vector<char>& buffer, const lt::info_hash_t& expected)
    {
        const lt::load_torrent_limits cfg{};

        lt::error_code ec;
        const lt::bdecode_node node = lt::bdecode(
            buffer,
            ec,
            nullptr,
            cfg.max_decode_depth,
            cfg.max_decode_tokens);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to decode info dict: " << ec;
            return nullptr;
        }

        auto ti = std::make_shared<lt::torrent_info>(node, ec, cfg, lt::from_info_section);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read info dict: " << ec;
            return nullptr;
        }

        const auto& actual = ti->info_hashes();

        if ((expected.has_v1() && actual.v1 != expected.v1)
            || (expected.has_v2() && actual.v2 != expected.v2))
        {
            BOOST_LOG_TRIVIAL(error) << "Info dict does not match info hash " << expected;
            return nullptr;
        }

        return ti;
    }

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

            if (client_data_json.contains("completed_at") && client_data_json.at("completed_at").is_number_integer())
            {
                client_data->completed_at = client_data_json["completed_at"].get<std::int64_t>();
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

        if (!params.ti)
        {
            const auto info_buffer = row.GetBuffer("info");

            if (!info_buffer.empty())
            {
                params.ti = ReadInfo(info_buffer, params.info_hashes);
            }
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
            {"category",     client_data.category.has_value()     ? json(client_data.category.value())     : json()},
            {"completed_at", client_data.completed_at.has_value() ? json(client_data.completed_at.value()) : json()},
            {"metadata",     client_data.metadata},
            {"tags",         client_data.tags}
        };

        return json(userdata).dump();
    }

    std::vector<char> WriteCleanResumeData(const lt::add_torrent_params& params)
    {
        lt::entry rd = lt::write_resume_data(params);
        rd.dict().erase("info");

        std::vector<char> buf;
        lt::bencode(std::back_inserter(buf), rd);

        return buf;
    }

    void WriteTorrentInfo(sqlite3* db, const int session_id, const lt::info_hash_t& hash, const lt::torrent_info& ti)
    {
        const auto              info_section = ti.info_section();
        const std::vector<char> info(info_section.begin(), info_section.end());

        auto stmt = Statement::Prepare(
            db,
            R"sql(
            INSERT OR IGNORE INTO torrentinfos (id, info)
            SELECT id, $info
            FROM addtorrentparams
            WHERE
                (session_id = $session_id AND info_hash_v1 = $info_hash_v1
                    AND (info_hash_v2 IS NULL OR info_hash_v2 = $info_hash_v2))
                OR
                (session_id = $session_id AND info_hash_v2 = $info_hash_v2
                    AND (info_hash_v1 IS NULL OR info_hash_v1 = $info_hash_v1))
            )sql");

        stmt
            .Bind("$info",         info)
            .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
            .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
            .Bind("$session_id",   session_id)
            .Execute();
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

    const std::vector<char> buf = WriteCleanResumeData(params);
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

    if (params.ti)
    {
        WriteTorrentInfo(db, session_id, hash, *params.ti);
    }
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
        atp.id             AS id,
        atp.queue_position AS queue_position,
        atp.params         AS params,
        atp.userdata       AS userdata,
        ti.info            AS info
    FROM
        addtorrentparams atp
    LEFT JOIN
        torrentinfos ti ON ti.id = atp.id
    WHERE
        atp.session_id = $session_id
        AND atp.queue_position <= $max_position
        AND atp.queue_position >= $position
        AND (atp.queue_position > $position OR atp.id > $id)
    ORDER BY
        atp.queue_position ASC, atp.id ASC
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
    auto info_stmt = Statement::Prepare(
        db,
        R"sql(
        DELETE FROM torrentinfos
        WHERE id IN (
            SELECT id FROM addtorrentparams
            WHERE
                (session_id = $session_id AND info_hash_v1 = $info_hash_v1
                    AND (info_hash_v2 IS NULL OR info_hash_v2 = $info_hash_v2))
                OR
                (session_id = $session_id AND info_hash_v2 = $info_hash_v2
                    AND (info_hash_v1 IS NULL OR info_hash_v1 = $info_hash_v1))
        )
        )sql");

    info_stmt
        .Bind("$info_hash_v1", hash.has_v1() ? std::optional(ToString(hash.v1)) : std::nullopt)
        .Bind("$info_hash_v2", hash.has_v2() ? std::optional(ToString(hash.v2)) : std::nullopt)
        .Bind("$session_id",   session_id)
        .Execute();

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

    const std::vector<char> buf = WriteCleanResumeData(params);

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

    if (params.ti)
    {
        WriteTorrentInfo(db, session_id, hash, *params.ti);
    }
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
