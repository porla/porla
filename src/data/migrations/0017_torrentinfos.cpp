#include "0017_torrentinfos.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/bdecode.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/torrent_info.hpp>

#include "../statement.hpp"

using porla::Data::Migrations::TorrentInfos;
using porla::Data::Statement;

namespace
{
    struct Row
    {
        std::int64_t      id;
        std::vector<char> params;
    };

    void MigrateInfoSection(sqlite3* db, const Row& row)
    {
        const lt::load_torrent_limits cfg{};

        lt::error_code ec;
        const lt::bdecode_node resume_data = lt::bdecode(
            row.params,
            ec,
            nullptr,
            cfg.max_decode_depth,
            cfg.max_decode_tokens);

        if (ec || resume_data.type() != lt::bdecode_node::dict_t)
        {
            BOOST_LOG_TRIVIAL(warning) << "Undecodable row found - ID " << row.id << ": " << ec;
            return;
        }

        const lt::bdecode_node info = resume_data.dict_find_dict("info");

        if (!info)
        {
            return;
        }

        const auto              info_section = info.data_section();
        const std::vector<char> info_buf(info_section.begin(), info_section.end());

        // the new params without the info section
        lt::entry params(resume_data);
        params.dict().erase("info");

        std::vector<char> params_buf;
        lt::bencode(std::back_inserter(params_buf), params);

        Statement::Prepare(db, "INSERT INTO torrentinfos (id, info) VALUES ($id, $info);")
            .Bind("$id",   row.id)
            .Bind("$info", info_buf)
            .Execute();

        Statement::Prepare(db, "UPDATE addtorrentparams SET params = $params WHERE id = $id;")
            .Bind("$id",     row.id)
            .Bind("$params", params_buf)
            .Execute();
    }
}

int TorrentInfos::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Adding table 'torrentinfos'";

    int res = sqlite3_exec(
        db,
        R"sql(
        CREATE TABLE torrentinfos (
            id   INTEGER PRIMARY KEY REFERENCES addtorrentparams(id),
            info BLOB NOT NULL
        );
        )sql",
        nullptr,
        nullptr,
        nullptr);

    if (res != SQLITE_OK)
    {
        return res;
    }

    BOOST_LOG_TRIVIAL(info) << "Moving info dicts to torrentinfos - might take a while";

    try
    {
        std::int64_t last_id = 0;

        while (true)
        {
            std::vector<Row> rows;

            Statement::Prepare(db, "SELECT id, params FROM addtorrentparams WHERE id > $id ORDER BY id LIMIT 100;")
                .Bind("$id", last_id)
                .Step([&](const Statement::IRow& row)
                {
                    rows.push_back({ row.GetInt64("id"), row.GetBuffer("params") });
                    return SQLITE_OK;
                });

            if (rows.empty())
            {
                break;
            }

            for (const auto& row : rows)
            {
                MigrateInfoSection(row);
                last_id = row.id;
            }
        }
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to move info dicts: " << e.what();
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}
