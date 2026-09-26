#pragma once

#include <sqlite3.h>

namespace porla::Data::Migrations
{
    struct AddTorrentParamsIndex
    {
        static int Migrate(sqlite3* db);
    };
}
