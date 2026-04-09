#include "../types.hpp"

#include <uWebSockets/App.h>

using porla::Lua::Types::UwsApp;

void UwsApp::Register(sol::state& lua)
{
    sol::table uws = lua["uws"].valid()
        ? lua["uws"].get<sol::table>()
        : lua.create_named_table("uws");

    uws["http_request"] = lua.new_usertype<uWS::HttpRequest>(
        "uws.http_request",
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

    uws["http_response"] = lua.new_usertype<uWS::HttpResponse<false>>(
        "uws.http_response",
        sol::no_constructor,
        "close", [](uWS::HttpResponse<false>* res) { res->close(); },
        "end", [](uWS::HttpResponse<false>* res, const std::string& body) {
            res->end(body);
        },
        "hasResponded", &uWS::HttpResponse<false>::hasResponded,
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

    uws["app"] = lua.new_usertype<uWS::App>(
        "uws.app",
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
        }
    );
}
