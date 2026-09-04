#pragma once

#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>

#include "responsewriter.hpp"

namespace porla::Rpc
{
    using Token = std::optional<jwt::decoded_jwt<jwt::traits::nlohmann_json>>;

    struct Method
    {
        virtual bool CanInvoke(Token token)
        {
            return token.has_value();
        }

        virtual void Invoke(
            const nlohmann::json& body,
            ResponseWriterHandle writer) = 0;
    };
}
