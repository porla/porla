#include "keyvalueset.hpp"

#include "../data/models/keyvaluestore.hpp"

using porla::Data::Models::KeyValueStore;
using porla::Methods::KeyValueSet;
using porla::Methods::KeyValueSetReq;
using porla::Methods::KeyValueSetRes;

KeyValueSet::KeyValueSet(sqlite3* db)
    : m_db(db)
{
}

void KeyValueSet::Invoke(const KeyValueSetReq& req, WriteCb<KeyValueSetRes> cb)
{
    for (const auto& [ key, value ] : req.values)
    {
        KeyValueStore::Set(m_db, key, value);
    }

    cb.Ok(KeyValueSetRes{});
}
