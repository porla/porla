#pragma once

#include <map>
#include <memory>

#include <nlohmann/json.hpp>

#include "handler.hpp"

namespace porla::Http
{
    class JsonRpcHandler
    {
    public:
        explicit JsonRpcHandler(
            std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<false>*)>> methods);

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
