#include "transaction.hpp"

#include <exception>

#include <boost/log/trivial.hpp>

using porla::Data::Transaction;

Transaction::Transaction(sqlite3* db)
    : m_db(db)
    , m_active(sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr) == SQLITE_OK)
    , m_uncaught_exceptions(std::uncaught_exceptions())
{
    if (!m_active)
    {
        BOOST_LOG_TRIVIAL(warning)
            << "Failed to begin transaction: " << sqlite3_errmsg(m_db);
    }
}

Transaction::~Transaction()
{
    if (!m_active)
    {
        return;
    }

    if (std::uncaught_exceptions() > m_uncaught_exceptions)
    {
        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    const auto res = sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr);

    if (res != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error)
            << "Failed to commit transaction: " << sqlite3_errmsg(m_db);

        sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
}
