#pragma once

#include <filesystem>
#include <string>

#include <sqlite3.h>

namespace fs = std::filesystem;

namespace porla
{
    class WebUI
    {
    public:
        static void Download(sqlite3* db);
    };
}
