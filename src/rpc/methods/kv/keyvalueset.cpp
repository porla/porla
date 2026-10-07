#include "keyvalueset.hpp"

#include "../../../keyvalue.hpp"

using porla::KeyValue;

using porla::Rpc::Methods::Kv::KeyValueSet;
using porla::Rpc::Methods::Kv::KeyValueSetReq;
using porla::Rpc::Methods::Kv::KeyValueSetRes;

KeyValueSet::KeyValueSet(KeyValue& kv)
    : m_kv(kv)
{
}

void KeyValueSet::Execute(const KeyValueSetReq& req, ResponseWriterHandle cb)
{
    m_kv.Set(req.values);

    cb->Ok(KeyValueSetRes{});
}
