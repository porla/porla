#pragma once

#include <nlohmann/json.hpp>

#include "../auth/context.hpp"
#include "responsewriter.hpp"

namespace porla::Rpc
{
    struct Method
    {
        virtual ~Method() = default;

        virtual bool CanInvoke(const Auth::Context&)
        {
            return true;
        }

        virtual void Invoke(
            const Auth::Context& caller,
            const nlohmann::json& body,
            ResponseWriterHandle writer) = 0;
    };
}
