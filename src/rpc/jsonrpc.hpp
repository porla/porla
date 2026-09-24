#pragma once

#include <map>
#include <memory>
#include <optional>

#include <uWebSockets/App.h>

#include "method.hpp"

using json = nlohmann::json;

namespace porla::Auth
{
    class Authenticator;
}

namespace porla::Rpc
{
    class JsonRpc : public std::enable_shared_from_this<JsonRpc>
    {
    public:
        static std::shared_ptr<JsonRpc> Create(std::shared_ptr<Auth::Authenticator> authenticator);

        std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> HttpHandler();

        bool Register(const std::string& name, const std::shared_ptr<Method>& method)
        {
            return m_methods.try_emplace(name, method).second;
        }

        void Unregister(const std::string& name)
        {
            m_methods.erase(name);
        }

    private:
        explicit JsonRpc(std::shared_ptr<Auth::Authenticator> authenticator);

        std::shared_ptr<Auth::Authenticator>           m_authenticator;
        std::map<std::string, std::shared_ptr<Method>> m_methods;
    };
}
