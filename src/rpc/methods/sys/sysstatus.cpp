#include "sysstatus.hpp"

#include "../../../data/models/users.hpp"

using porla::Rpc::Methods::Sys::SysStatus;

SysStatus::SysStatus(sqlite3* db)
    : m_db(db)
{
}

void SysStatus::Execute(const nlohmann::json& req, ResponseWriterHandle cb)
{
    auto any_users = Data::Models::Users::Any(m_db);

    cb->Ok({
        {"status", any_users ? "ok" : "setup"}
    });
}
