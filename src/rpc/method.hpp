#pragma once

#include <nlohmann/json.hpp>

#include "../auth/context.hpp"
#include "responsewriter.hpp"

namespace porla::Rpc
{
    struct Method
    {
        virtual bool CanInvoke(const Auth::Context& auth_ctx)
        {
            return auth_ctx.IsAuthenticated();
        }

        virtual void Invoke(
            const nlohmann::json& body,
            ResponseWriterHandle writer) = 0;
    };
}
