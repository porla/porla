#pragma once

#include <memory>
#include <sqlite3.h>

namespace porla
{
    class Config;
}

namespace porla::Data::Migrations
{
    struct MorePresets
    {
        static int Migrate(sqlite3* db);
    };
}
