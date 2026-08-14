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
        static std::shared_ptr<JsonRpc> Create(const std::string& secret_key);

        std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> HttpHandler();

        void Register(const std::string& name, const std::shared_ptr<Method>& method)
        {
            m_methods[name] = method;
        }

    private:
        explicit JsonRpc(const std::string& secret_key);

        std::map<std::string, std::shared_ptr<Method>> m_methods;
        std::string                                    m_secret_key;
    };
}
