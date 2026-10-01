#include "authkeysremove.hpp"

using porla::Rpc::Methods::Auth::AuthKeysRemove;
using porla::Rpc::Methods::Auth::AuthKeysRemoveReq;
using porla::Rpc::Methods::Auth::AuthKeysRemoveRes;

AuthKeysRemove::AuthKeysRemove(sqlite3* db)
    : m_db(db)
{
}

void AuthKeysRemove::Execute(const AuthKeysRemoveReq& req, ResponseWriterHandle cb)
{
}