#include "transaction.hpp"

#include <exception>
#include <stdexcept>
#include <string>

#include "../logger.hpp"

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("db"); }
}

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
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("error", sqlite3_errmsg(m_db))
            << "Failed to commit transaction";

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
        BOOST_LOG_SEV(logger::get(), Log::error)
            << Log::Property("error", sqlite3_errmsg(m_db))
            << "Failed to rollback transaction";
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
        BOOST_LOG_SEV(logger::get(), Log::error)
            << "Transaction destroyed without commit or rollback - rolling back";
    }

    Rollback();
}
