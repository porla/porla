#pragma once

#include <map>
#include <memory>
#include <optional>

#include <uWebSockets/App.h>

#include "method.hpp"

using json = nlohmann::json;

namespace porla::Rpc
{
    class JsonRpc : public std::enable_shared_from_this<JsonRpc>
    {
    public:
        static std::shared_ptr<JsonRpc> Create();

        std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> HttpHandler();

        void Register(const std::string& name, const std::shared_ptr<Method>& method)
        {
            m_methods[name] = method;
        }

    private:
        std::map<std::string, std::shared_ptr<Method>> m_methods;
    };
}
