#include "authkeyslist.hpp"

#include "../../../data/models/apikeys.hpp"

using porla::Data::Models::ApiKeys;
using porla::Rpc::Methods::Auth::AuthKeysList;
using porla::Rpc::Methods::Auth::AuthKeysListReq;
using porla::Rpc::Methods::Auth::AuthKeysListRes;

AuthKeysList::AuthKeysList(sqlite3* db)
    : m_db(db)
{
}

void AuthKeysList::Execute(const AuthKeysListReq& req, ResponseWriterHandle cb)
{
    const auto keys = ApiKeys::List(m_db);

    std::vector<AuthKeysListRes::Key> output_keys;
    output_keys.reserve(keys.size());

    std::transform(
        keys.begin(),
        keys.end(),
        std::back_inserter(output_keys),
        [](const ApiKeys::ApiKey& model)
        {
            return AuthKeysListRes::Key{
                .id         = model.id,
                .name       = model.name,
                .created_at = model.created_at,
                .expires_at = model.expires_at
            };
        });

    cb->Ok(AuthKeysListRes{
        .keys = output_keys
    });
}
