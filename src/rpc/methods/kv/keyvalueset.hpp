#pragma once

#include "../../typedmethod.hpp"

#include "keyvalueset_reqres.hpp"

namespace porla
{
    class KeyValue;
}

namespace porla::Rpc::Methods::Kv
{
    class KeyValueSet : public TypedMethod<KeyValueSetReq, KeyValueSetRes>
    {
    public:
        explicit KeyValueSet(KeyValue& kv);

    protected:
        bool CanInvoke(const porla::Auth::Context& auth_ctx) override
        {
            return auth_ctx.IsAuthenticated() && auth_ctx.kind == porla::Auth::Context::Kind::User;
        }

        void Execute(const KeyValueSetReq& req, ResponseWriterHandle cb) override;

    private:
        KeyValue& m_kv;
    };
}
