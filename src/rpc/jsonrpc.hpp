#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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

        bool Register(const std::string& name, std::vector<Auth::Context::Kind> kinds, const std::shared_ptr<Method>& method)
        {
            return m_methods.try_emplace(name, Entry{ .kinds = std::move(kinds), .method = method }).second;
        }

        void Unregister(const std::string& name)
        {
            m_methods.erase(name);
        }

    private:
        struct Entry
        {
            std::vector<Auth::Context::Kind> kinds;
            std::shared_ptr<Method>          method;

            bool Allows(const Auth::Context& ctx) const
            {
                return std::ranges::find(kinds, ctx.kind) != kinds.end();
            }
        };

        explicit JsonRpc(std::shared_ptr<Auth::Authenticator> authenticator);

        std::shared_ptr<Auth::Authenticator> m_authenticator;
        std::map<std::string, Entry>         m_methods;
    };
}
