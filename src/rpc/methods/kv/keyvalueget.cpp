#include "keyvalueget.hpp"

#include "../../../keyvalue.hpp"

using porla::KeyValue;

using porla::Rpc::Methods::Kv::KeyValueGet;
using porla::Rpc::Methods::Kv::KeyValueGetReq;
using porla::Rpc::Methods::Kv::KeyValueGetRes;

KeyValueGet::KeyValueGet(const KeyValue& kv)
    : m_kv(kv)
{
}

void KeyValueGet::Execute(const KeyValueGetReq& req, ResponseWriterHandle cb)
{
    std::map<std::string, nlohmann::json> values;

    for (const auto& key : req.keys)
    {
        values.insert({ key, m_kv.Get(key) });
    }

    cb->Ok(KeyValueGetRes{
        .values = values
    });
}
