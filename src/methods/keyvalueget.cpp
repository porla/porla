#include "keyvalueget.hpp"

#include "../data/models/keyvaluestore.hpp"

using porla::Data::Models::KeyValueStore;
using porla::Methods::KeyValueGet;
using porla::Methods::KeyValueGetReq;
using porla::Methods::KeyValueGetRes;

KeyValueGet::KeyValueGet(sqlite3* db)
    : m_db(db)
{
}

void KeyValueGet::Invoke(const KeyValueGetReq& req, WriteCb<KeyValueGetRes> cb)
{
    std::map<std::string, nlohmann::json> values;

    for (const auto& key : req.keys)
    {
        values.insert({ key, KeyValueStore::Get(m_db, key) });
    }

    cb.Ok(KeyValueGetRes{
        .values = values
    });
}
