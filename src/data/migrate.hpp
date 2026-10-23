#pragma once

#include <sqlite3.h>

namespace porla::Data
{
    bool Migrate(sqlite3* db);
}
