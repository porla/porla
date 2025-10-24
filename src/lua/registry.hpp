#pragma once

#include <sqlite3.h>

namespace porla::Lua::Registry
{
    struct Sqlite3
    {
        sqlite3* db;
    };
}
