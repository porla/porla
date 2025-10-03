#include "0012_alterplugins.hpp"

#include <boost/log/trivial.hpp>

using porla::Data::Migrations::AlterPlugins;

int AlterPlugins::Migrate(sqlite3* db)
{
    BOOST_LOG_TRIVIAL(info) << "Altering 'plugins' table";

    int res = sqlite3_exec(
        db,
        "CREATE TEMPORARY TABLE plugins_backup (id, path, config);\n"

        "INSERT INTO plugins_backup SELECT id,path,config FROM plugins;\n"

        "DROP TABLE plugins;\n"

        "CREATE TABLE plugins (id INTEGER PRIMARY KEY, type TEXT NOT NULL, data BLOB NOT NULL, config TEXT, metadata TEXT)\n;"

        "INSERT INTO plugins (id, type, data, config, metadata)\n"
        "SELECT pb.id,\"path\",pb.path,pb.config,NULL\n"
        "FROM plugins_backup pb;\n"

        "DROP TABLE plugins_backup;\n",
        nullptr,
        nullptr,
        nullptr);

    return res;
}
