#pragma once

#include <sqlite3.h>

namespace porla::Data::Migrations
{
    struct RemoveSessionParams
    {
        static int Migrate(sqlite3* db);
    };
}
