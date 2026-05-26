#include "../types.hpp"

#include <uWebSockets/App.h>

#include "../host.hpp"

using porla::Lua::Types::HttpServer;

void HttpServer::Register(sol::state& lua)
{
    sol::table http = lua["http"].valid()
        ? lua["http"].get<sol::table>()
        : lua.create_named_table("http");

    http["HttpRequest"] = lua.new_usertype<uWS::HttpRequest>(
        "http.HttpRequest",
        sol::no_constructor,
        "getCaseSensitiveMethod", &uWS::HttpRequest::getCaseSensitiveMethod,
        "getFullUrl", &uWS::HttpRequest::getFullUrl,
        "getHeader", &uWS::HttpRequest::getHeader,
        "getMethod", &uWS::HttpRequest::getMethod,
        "getParameter", sol::overload(
            [](uWS::HttpRequest* req, int index)              { return req->getParameter(index); },
            [](uWS::HttpRequest* req, const std::string name) { return req->getParameter(name); }
        ),
        "getQuery", sol::overload(
            [](uWS::HttpRequest* req)                         { return req->getQuery(); },
            [](uWS::HttpRequest* req, const std::string& key) { return req->getQuery(key); }
        ),
        "getUrl", &uWS::HttpRequest::getUrl,
        "getYield", &uWS::HttpRequest::getYield
    );

    http["HttpResponse"] = lua.new_usertype<uWS::HttpResponse<false>>(
        "http.HttpResponse",
        sol::no_constructor,
        "close", [](uWS::HttpResponse<false>* res) { res->close(); },
        "endr", [](uWS::HttpResponse<false>* res, const std::string& body) {
            res->end(body);
        },
        "hasResponded", &uWS::HttpResponse<false>::hasResponded,
        "onAborted", [](uWS::HttpResponse<false>* res, sol::protected_function callback)
        {
            auto cb = std::make_shared<sol::protected_function>(std::move(callback));
            res->onAborted([cb]() { (*cb)(); });
        },
        "onData", [](sol::this_state L, uWS::HttpResponse<false>* res, sol::protected_function callback)
        {
            sol::state_view lua(L);

            auto cb = std::make_shared<sol::protected_function>(std::move(callback));
            auto host = lua.registry()["host"].get<porla::Lua::Host*>();

            res->onData([cb, host](std::string_view data, bool fin)
            {
                std::string data_owned(data);
                host->SpawnCoroutine(*cb, data_owned, fin);
            });
        },
        "pause", [](uWS::HttpResponse<false>* res) { res->pause(); },
        "resume", [](uWS::HttpResponse<false>* res) { res->resume(); },
        "write", &uWS::HttpResponse<false>::write,
        "writeContinue", &uWS::HttpResponse<false>::writeContinue,
        "writeHeader", sol::overload(
            [](uWS::HttpResponse<false>* res, const std::string& key, uint64_t value)           { res->writeHeader(key, value); },
            [](uWS::HttpResponse<false>* res, const std::string& key, const std::string& value) { res->writeHeader(key, value); }
        ),
        "writeStatus", &uWS::HttpResponse<false>::writeStatus
    );

    http["Server"] = lua.new_usertype<uWS::App>(
        "http.Server",
        sol::call_constructor, sol::factories(
            []() { return std::make_shared<uWS::App>(); }
        ),
        "get", [](std::shared_ptr<uWS::App> app, const std::string& path, sol::protected_function callback)
        {
            auto cb = std::make_shared<sol::protected_function>(std::move(callback));

            app->get(path, [cb](uWS::HttpResponse<false>* response, uWS::HttpRequest* request)
            {
                (*cb)(response, request);
            });
        },
        "listen", [](std::shared_ptr<uWS::App> app, const std::string& host, int port, sol::protected_function callback)
        {
            auto cb = std::make_shared<sol::protected_function>(std::move(callback));

            app->listen(host, port, [cb](const auto* t)
            {
                (*cb)();
            });
        },
        "post", [](std::shared_ptr<uWS::App> app, const std::string& path, sol::protected_function callback)
        {
            auto cb = std::make_shared<sol::protected_function>(std::move(callback));

            app->post(path, [cb](uWS::HttpResponse<false>* response, uWS::HttpRequest* request)
            {
                (*cb)(response, request);
            });
        }
    );
}
