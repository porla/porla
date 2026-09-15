#pragma once

#include "method.hpp"

namespace porla::Rpc
{
    template<typename TReq, typename TRes>
    class TypedMethod : public Method
    {
    public:
        void Invoke(const nlohmann::json& body, ResponseWriterHandle writer) override
        {
            return Execute(body.get<TReq>(), std::move(writer));
        }

        virtual void Execute(const TReq& req, ResponseWriterHandle writer) = 0;
    };
}
