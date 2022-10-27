#pragma once

#include <map>
#include <memory>

#include <nlohmann/json.hpp>

#include "httpcontext.hpp"

namespace porla
{
    class JsonRpcHandler
    {
    public:
        explicit JsonRpcHandler(
            std::map<std::string, std::function<void(const nlohmann::json&, std::shared_ptr<porla::HttpContext>)>> methods);

        JsonRpcHandler(const JsonRpcHandler&) = delete;
        JsonRpcHandler& operator=(const JsonRpcHandler&) = delete;
        JsonRpcHandler& operator=(JsonRpcHandler&&) = delete;

        void operator()(const std::shared_ptr<porla::HttpContext>& ctx);

    private:
        std::map<std::string, std::function<void(const nlohmann::json&, std::shared_ptr<porla::HttpContext>)>> m_methods;
    };
}
