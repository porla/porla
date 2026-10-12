#include "transaction.hpp"

#include <exception>
#include <stdexcept>
#include <string>

#include <boost/log/trivial.hpp>

using porla::Data::Transaction;

Transaction::Transaction(sqlite3* db)
    : m_db(db)
    , m_active(sqlite3_exec(db, "BEGIN;", nullptr, nullptr, nullptr) == SQLITE_OK)
    , m_uncaught_exceptions(std::uncaught_exceptions())
{
    if (!m_active)
    {
        throw std::runtime_error("Failed to begin transaction: " + std::string(sqlite3_errmsg(db)));
    }
}

bool Transaction::Commit()
{
    if (!m_active)
    {
        return false;
    }

    if (sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "db[tx] Failed to commit transaction: " << sqlite3_errmsg(m_db);
        Rollback();
        return false;
    }

    m_active = false;

    return true;
}

void Transaction::Rollback()
{
    if (!m_active)
    {
        return;
    }

    m_active = false;

    const auto in_transaction = sqlite3_get_autocommit(m_db) == 0;

    if (in_transaction && sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "db[tx] Failed to roll back transaction: " << sqlite3_errmsg(m_db);
    }
}

Transaction::~Transaction()
{
    if (!m_active)
    {
        return;
    }

    if (std::uncaught_exceptions() <= m_uncaught_exceptions)
    {
        BOOST_LOG_TRIVIAL(error) << "db[tx] Transaction destroyed without commit or rollback - rolling back";
    }

    Rollback();
}
