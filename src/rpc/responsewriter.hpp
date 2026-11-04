#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "../errors.hpp"

namespace porla::Rpc
{
    class ResponseWriter
    {
    public:
        virtual ~ResponseWriter() {}

        virtual void Error(int code, const std::string& message, const nlohmann::json& data = {}) = 0;
        virtual void Header(const std::string& name, const std::string& value) = 0;
        virtual void Ok(const nlohmann::json& j) = 0;
        void Ok() { Ok(nullptr); }

        // data.reason is the registry name; message defaults to the registry's
        void Error(Errors::Code code, const std::string& message = {}, nlohmann::json data = nlohmann::json::object())
        {
            const auto entry = Errors::Get(code);

            data["reason"] = entry.name;

            Error(entry.rpc_code, message.empty() ? std::string(entry.message) : message, data);
        }
    };

    using ResponseWriterHandle = std::shared_ptr<ResponseWriter>;
}
