#pragma once

#include <sqlite3.h>

namespace porla::Data
{
    class Transaction
    {
    public:
        explicit Transaction(sqlite3* db);
        ~Transaction();

        Transaction(const Transaction&)            = delete;
        Transaction& operator=(const Transaction&) = delete;

    private:
        sqlite3* m_db;
        bool     m_active;
        int      m_uncaught_exceptions;
    };
}
