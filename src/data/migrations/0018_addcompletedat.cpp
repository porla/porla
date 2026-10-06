#include "0018_addcompletedat.hpp"

#include <ctime>

#include <boost/log/trivial.hpp>
#include <libtorrent/bdecode.hpp>
#include <nlohmann/json.hpp>

#include "../statement.hpp"

using json = nlohmann::json;
using porla::Data::Migrations::AddCompletedAt;
using porla::Data::Statement;

namespace
{
    struct Row
    {
        std::int64_t      id;
        std::vector<char> params;
        std::string       userdata;
    };

    // best available "first completed" time: libtorrent's completed_time, then added_time, then now
    std::int64_t CompletedTime(const Row& row)
    {
        lt::error_code ec;
        const auto rd = lt::bdecode(row.params, ec);

        if (!ec && rd.type() == lt::bdecode_node::dict_t)
        {
            if (const auto t = rd.dict_find_int_value("completed_time", 0); t > 0) { return t; }
            if (const auto t = rd.dict_find_int_value("added_time",     0); t > 0) { return t; }
        }

        return static_cast<std::int64_t>(std::time(nullptr));
    }

    void MigrateRow(sqlite3* db, const Row& row)
    {
        json userdata = json::parse(row.userdata, nullptr, false);

        if (userdata.is_discarded() || !userdata.is_object())
        {
            BOOST_LOG_TRIVIAL(warning) << "Unparsable userdata found - ID " << row.id;
            return;
        }

        auto metadata = userdata.find("metadata");

        if (metadata == userdata.end() || !metadata->is_object() || !metadata->contains("signal:finished"))
        {
            return;
        }

        if (metadata->at("signal:finished") == true)
        {
            userdata["completed_at"] = CompletedTime(row);
        }

        metadata->erase("signal:finished");

        Statement::Prepare(db, "UPDATE addtorrentparams SET userdata = $userdata WHERE id = $id;")
            .Bind("$id",       row.id)
            .Bind("$userdata", userdata.dump())
            .Execute();
    }
}

int AddCompletedAt::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Moving 'signal:finished' markers to completed_at";

    try
    {
        std::int64_t last_id = 0;

        while (true)
        {
            std::vector<Row> rows;

            Statement::Prepare(
                db,
                R"sql(
                SELECT id, params, userdata
                FROM addtorrentparams
                WHERE id > $id AND userdata LIKE '%signal:finished%'
                ORDER BY id LIMIT 100;
                )sql")
                .Bind("$id", last_id)
                .Step([&](const Statement::IRow& row)
                {
                    rows.push_back({ row.GetInt64("id"), row.GetBuffer("params"), row.GetStdString("userdata") });
                    return SQLITE_OK;
                });

            if (rows.empty())
            {
                break;
            }

            for (const auto& row : rows)
            {
                MigrateRow(db, row);
                last_id = row.id;
            }
        }
    }
    catch (const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to migrate finished markers: " << e.what();
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}
