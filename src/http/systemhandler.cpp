#include "systemhandler.hpp"

#include <nlohmann/json.hpp>

#include "../data/models/users.hpp"

using json = nlohmann::json;
using porla::Http::SystemHandler;

SystemHandler::SystemHandler(sqlite3* db)
    : m_db(db)
{
}

void SystemHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    auto any_users = porla::Data::Models::Users::Any(m_db);

    json j = {
        {"status", any_users ? "ok" : "setup"}
    };

    res->writeHeader("Content-Type", "application/json")
        ->writeStatus("200 OK")
        ->end(j.dump());
}
