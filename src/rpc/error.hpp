#pragma once

#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

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

        int            code;
        nlohmann::json data;
    };
}
