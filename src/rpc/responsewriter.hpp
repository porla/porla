#pragma once

#include <memory>

#include <nlohmann/json.hpp>

#include "../json/all.hpp"

namespace porla::Rpc
{
    class ResponseWriter
    {
    public:
        virtual void Error(int code, const std::string& message, const nlohmann::json& data = {}) = 0;
        virtual void Header(const std::string& name, const std::string& value) = 0;
        virtual void Ok(const nlohmann::json& j) = 0;
        virtual void Write(const nlohmann::json& j) = 0;
    };

    using ResponseWriterHandle = std::unique_ptr<ResponseWriter>;
}
