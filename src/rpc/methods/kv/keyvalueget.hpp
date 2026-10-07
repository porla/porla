#pragma once

#include "../../typedmethod.hpp"

#include "keyvalueget_reqres.hpp"

namespace porla
{
    class KeyValue;
}

namespace porla::Rpc::Methods::Kv
{
    class KeyValueGet : public TypedMethod<KeyValueGetReq, KeyValueGetRes>
    {
    public:
        explicit KeyValueGet(const KeyValue& kv);

    protected:
        void Execute(const KeyValueGetReq& req, ResponseWriterHandle cb) override;

    private:
        const KeyValue& m_kv;
    };
}
