#include "authkeysremove.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/apikeys.hpp"

using porla::Data::Models::ApiKeys;
using porla::Rpc::Methods::Auth::AuthKeysRemove;
using porla::Rpc::Methods::Auth::AuthKeysRemoveReq;
using porla::Rpc::Methods::Auth::AuthKeysRemoveRes;

AuthKeysRemove::AuthKeysRemove(sqlite3* db)
    : m_db(db)
{
}

void AuthKeysRemove::Execute(const AuthKeysRemoveReq& req, ResponseWriterHandle cb)
{
    ApiKeys::Remove(m_db, req.id);

    BOOST_LOG_TRIVIAL(info) << "API key " << req.id << " removed";

    cb->Ok({});
}
