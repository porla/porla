#include "authkeyslist.hpp"

using porla::Rpc::Methods::Auth::AuthKeysList;
using porla::Rpc::Methods::Auth::AuthKeysListReq;
using porla::Rpc::Methods::Auth::AuthKeysListRes;

AuthKeysList::AuthKeysList(sqlite3* db)
    : m_db(db)
{
}

void AuthKeysList::Execute(const AuthKeysListReq& req, ResponseWriterHandle cb)
{
}
