#pragma once

#include <map>
#include <memory>

#include <nlohmann/json.hpp>

#include "handler.hpp"

namespace porla::Http
{
    template <bool SSL> class JsonRpcHandler
    {
    public:
        explicit JsonRpcHandler(
            std::map<std::string, std::function<void(const nlohmann::json&, const nlohmann::json&, uWS::HttpResponse<SSL>*)>> methods);

        void operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req);

    private:
        template <bool SSLS> class State;
        std::shared_ptr<State<SSL>> m_state;
    };
}
