#pragma once

#include <memory>
#include <sqlite3.h>

namespace porla
{
    class Config;
}

namespace porla::Data::Migrations
{
    struct Sessions
    {
        static int Migrate(sqlite3* db, const std::unique_ptr<Config>& cfg);
    };
}
