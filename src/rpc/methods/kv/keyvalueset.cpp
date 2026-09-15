#include "keyvalueset.hpp"

#include <boost/asio.hpp>

#include "../../../data/models/keyvaluestore.hpp"

using porla::Data::Models::KeyValueStore;

using porla::Rpc::Methods::Kv::KeyValueSet;
using porla::Rpc::Methods::Kv::KeyValueSetReq;
using porla::Rpc::Methods::Kv::KeyValueSetRes;

KeyValueSet::KeyValueSet(
    boost::asio::io_context& io,
    sqlite3* db,
    boost::signals2::signal<void(const std::unordered_set<std::string>&)>& kv_updated)
    : m_io(io)
    , m_db(db)
    , m_kv_updated(kv_updated)
{
}

void KeyValueSet::Execute(const KeyValueSetReq& req, ResponseWriterHandle cb)
{
    std::unordered_set<std::string> updated_keys;

    for (const auto& [ key, value ] : req.values)
    {
        KeyValueStore::Set(m_db, key, value);

        updated_keys.insert(key);
    }

    cb->Ok(KeyValueSetRes{});

    boost::asio::post(m_io, [this, updated_keys]() { m_kv_updated(updated_keys); });
}
