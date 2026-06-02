#include "sysstatus.hpp"

#include "../data/models/users.hpp"

using porla::Methods::SysStatus;

SysStatus::SysStatus(sqlite3* db)
    : m_db(db)
{
}

void SysStatus::Invoke(const json &req, WriteCb<std::map<std::string, json>> cb)
{
    auto any_users = porla::Data::Models::Users::Any(m_db);

    cb.Ok({
        {"status", any_users ? "ok" : "setup"}
    });
}
