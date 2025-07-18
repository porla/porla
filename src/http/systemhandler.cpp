#include "systemhandler.hpp"

#include <nlohmann/json.hpp>

#include "../data/models/users.hpp"

using json = nlohmann::json;
using porla::Http::SystemHandler;

template <bool SSL> SystemHandler<SSL>::SystemHandler(sqlite3* db)
    : m_db(db)
{
}

template <bool SSL> void SystemHandler<SSL>::operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req)
{
    auto any_users = porla::Data::Models::Users::Any(m_db);

    json j = {
        {"status", any_users ? "ok" : "setup"}
    };

    res->writeHeader("Content-Type", "application/json")
        ->writeStatus("200 OK")
        ->end(j.dump());
}

namespace porla::Http
{
    template class SystemHandler<true>;
    template class SystemHandler<false>;
}
