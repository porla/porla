#include "systemhandler.hpp"

#include "data/models/users.hpp"

using porla::SystemHandler;

SystemHandler::SystemHandler(sqlite3* db)
    : m_db(db)
{
}

void SystemHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    auto any_users = porla::Data::Models::Users::Any(m_db);

    ctx->WriteJson({
        {"status", any_users ? "ok" : "setup"}
    });
}
