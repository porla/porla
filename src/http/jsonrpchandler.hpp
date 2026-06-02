#pragma once

#include <map>
#include <memory>

#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <uWebSockets/App.h>

namespace porla::Http
{
    class JsonRpcHandler
    {
    public:
        explicit JsonRpcHandler(
            std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*, std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>)>> methods);

        void operator()(
            uWS::HttpResponse<false>* res,
            uWS::HttpRequest* req);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
