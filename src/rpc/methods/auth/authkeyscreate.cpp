#include "authkeyscreate.hpp"

#include <boost/log/trivial.hpp>
#include <sodium.h>

#include "../../../data/models/apikeys.hpp"

using porla::Data::Models::ApiKeys;
using porla::Rpc::Methods::Auth::AuthKeysCreate;
using porla::Rpc::Methods::Auth::AuthKeysCreateReq;
using porla::Rpc::Methods::Auth::AuthKeysCreateRes;

AuthKeysCreate::AuthKeysCreate(sqlite3* db)
    : m_db(db)
{
}

void AuthKeysCreate::Execute(const AuthKeysCreateReq& req, ResponseWriterHandle cb)
{
    unsigned char key_id_bin[8];
    char          key_id[sizeof(key_id_bin) * 2 + 1];
    unsigned char key_secret[32];

    randombytes_buf(key_id_bin, sizeof(key_id_bin));
    randombytes_buf(key_secret, sizeof(key_secret));

    sodium_bin2hex(
        key_id,
        sizeof(key_id),
        key_id_bin,
        sizeof(key_id_bin));

    const auto secret_encoded_len = sodium_base64_ENCODED_LEN(
        sizeof(key_secret),
        sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    std::string secret_encoded;
    secret_encoded.resize(secret_encoded_len);

    sodium_bin2base64(
        secret_encoded.data(),
        secret_encoded.size(),
        key_secret,
        sizeof(key_secret),
        sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    secret_encoded.resize(secret_encoded_len - 1);

    unsigned char secret_hashed[crypto_generichash_BYTES];

    crypto_generichash(
        secret_hashed,
        sizeof(secret_hashed),
        key_secret,
        sizeof(key_secret),
        nullptr,
        0);

    sodium_memzero(key_secret, sizeof(key_secret));

    ApiKeys::Insert(
        m_db,
        key_id,
        req.name,
        std::vector<char>(
            reinterpret_cast<char*>(secret_hashed),
            reinterpret_cast<char*>(secret_hashed) + sizeof(secret_hashed)),
        req.expires_at);

    BOOST_LOG_TRIVIAL(info) << "New API key created";

    cb->Ok(AuthKeysCreateRes{
        .id     = key_id,
        .secret = secret_encoded
    });
}
