#pragma once

#include <memory>

#include <sqlite3.h>

namespace porla
{
    class Config;
}

namespace porla::Data::Migrations
{
    // 0001
    int InitialSetup(sqlite3* db);

    // 0002
    int AddSessionSettings(sqlite3* db);

    // 0003
    int Users(sqlite3* db);

    // 0004
    int RemoveSessionParams(sqlite3* db);

    // 0005
    int TorrentsMetadata(sqlite3* db);

    // 0006
    int ClientData(sqlite3* db);

    // 0007
    int RemoveSessionSettings(sqlite3* db);

    // 0008
    int Plugins(sqlite3* db);

    // 0009
    int MultiSessions(sqlite3* db);

    // 0010
    int Sessions(sqlite3* db, const std::unique_ptr<Config>& config);

    // 0011
    int Presets(sqlite3* db, const std::unique_ptr<Config>& config);

    // 0012
    int AlterPlugins(sqlite3* db);

    // 0013
    int AddKeyValueStore(sqlite3* db);

    // 0014
    int MorePresets(sqlite3* db);

    // 0015
    int AddTorrentParamsIndex(sqlite3* db);

    // 0016
    int AddApiKeys(sqlite3* db);

    // 0017
    int TorrentInfos(sqlite3* db);

    // 0018
    int AddCompletedAt(sqlite3* db);

    // 0019
    int AddAutoIncrementIds(sqlite3* db);
}
