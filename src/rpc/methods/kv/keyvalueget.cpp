#include "keyvalueget.hpp"

#include "../../../data/models/keyvaluestore.hpp"

using porla::Data::Models::KeyValueStore;

using porla::Rpc::Methods::Kv::KeyValueGet;
using porla::Rpc::Methods::Kv::KeyValueGetReq;
using porla::Rpc::Methods::Kv::KeyValueGetRes;

KeyValueGet::KeyValueGet(sqlite3* db)
    : m_db(db)
{
}

void KeyValueGet::Execute(const KeyValueGetReq& req, ResponseWriterHandle cb)
{
    std::map<std::string, nlohmann::json> values;

    for (const auto& key : req.keys)
    {
        values.insert({ key, KeyValueStore::Get(m_db, key) });
    }

    cb->Ok(KeyValueGetRes{
        .values = values
    });
}
