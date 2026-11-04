#pragma once

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "../errors.hpp"

namespace porla::Rpc
{
    // Thrown by a method function to reply with a JSON-RPC error.
    struct Error : std::runtime_error
    {
        Error(int code, const std::string& message, nlohmann::json data = nullptr)
            : std::runtime_error(message)
            , code(code)
            , data(std::move(data))
        {
        }

        Error(
            Errors::Code code,
            const std::string& message = {},
            nlohmann::json data = nlohmann::json::object())
            : Error(
                Errors::Get(code).rpc_code,
                message.empty() ? std::string(Errors::Get(code).message) : message,
                std::move(data))
        {
            this->data["reason"] = Errors::Get(code).name;
        }

        int            code;
        nlohmann::json data;
    };
}
