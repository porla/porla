#include "0012_alterplugins.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::AlterPlugins;

int AlterPlugins::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Altering 'plugins' table";

    int res = sqlite3_exec(
        db,
        R"sql(
        CREATE TEMPORARY TABLE plugins_backup (id, path, config);

        INSERT INTO plugins_backup
        SELECT id,path,config FROM plugins;

        DROP TABLE plugins;

        CREATE TABLE plugins(
            id INTEGER PRIMARY KEY,
            path TEXT NOT NULL,
            config TEXT,
            metadata TEXT
        );

        INSERT INTO plugins (id, path, config, metadata)
        SELECT pb.id,pb.path,pb.config,NULL
        FROM plugins_backup pb;

        DROP TABLE plugins_backup;
        )sql",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
