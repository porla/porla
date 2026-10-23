#include "tokens.hpp"

#include <ctime>

#include <boost/log/trivial.hpp>
#include <sodium.h>

using TokenRows = porla::Data::Models::Tokens;

namespace
{
    // porla_<16 hex>_<43 base64url>
    constexpr size_t kIdBytes     = 8;
    constexpr size_t kIdLen       = kIdBytes * 2;
    constexpr size_t kSecretBytes = 32;
    constexpr size_t kSecretLen   = 43;
    constexpr size_t kTokenLen    = 6 + kIdLen + 1 + kSecretLen;

    std::vector<char> Hash(const unsigned char* secret, size_t len)
    {
        unsigned char hashed[crypto_generichash_BYTES];
        crypto_generichash(hashed, sizeof(hashed), secret, len, nullptr, 0);

        return { reinterpret_cast<char*>(hashed), reinterpret_cast<char*>(hashed) + sizeof(hashed) };
    }
}

porla::Auth::Tokens::Issued porla::Auth::Tokens::Issue(sqlite3* db, Data::Models::Tokens::Token row)
{
    unsigned char id_bin[kIdBytes];
    char          id[kIdLen + 1];
    unsigned char secret[kSecretBytes];

    randombytes_buf(id_bin, sizeof(id_bin));
    randombytes_buf(secret, sizeof(secret));

    sodium_bin2hex(id, sizeof(id), id_bin, sizeof(id_bin));

    std::string secret_encoded;
    secret_encoded.resize(
        sodium_base64_ENCODED_LEN(sizeof(secret),
        sodium_base64_VARIANT_URLSAFE_NO_PADDING));

    sodium_bin2base64(
        secret_encoded.data(),
        secret_encoded.size(),
        secret,
        sizeof(secret),
        sodium_base64_VARIANT_URLSAFE_NO_PADDING);

    secret_encoded.resize(kSecretLen);

    row.id          = id;
    row.secret_hash = Hash(secret, sizeof(secret));
    row.created_at  = std::time(nullptr);

    sodium_memzero(secret, sizeof(secret));

    TokenRows::Insert(db, row);

    return Issued{
        .id    = row.id,
        .token = "porla_" + row.id + "_" + secret_encoded
    };
}

std::optional<TokenRows::Token> porla::Auth::Tokens::Verify(sqlite3* db, std::string_view token)
{
    if (token.size() != kTokenLen || !token.starts_with("porla_") || token[6 + kIdLen] != '_')
    {
        return std::nullopt;
    }

    const auto id             = token.substr(6, kIdLen);
    const auto secret_encoded = token.substr(6 + kIdLen + 1);

    unsigned char secret[kSecretBytes];
    size_t        secret_len = 0;

    if (sodium_base642bin(
            secret,
            sizeof(secret),
            secret_encoded.data(),
            secret_encoded.size(),
            nullptr,
            &secret_len,
            nullptr,
            sodium_base64_VARIANT_URLSAFE_NO_PADDING) != 0
        || secret_len != sizeof(secret))
    {
        return std::nullopt;
    }

    const auto hashed = Hash(secret, sizeof(secret));
    sodium_memzero(secret, sizeof(secret));

    auto row = TokenRows::GetById(db, id);

    const auto now = static_cast<std::int64_t>(std::time(nullptr));

    if (!row.has_value()
        || (row->expires_at.has_value() && *row->expires_at < now)
        || row->secret_hash.size() != hashed.size()
        || sodium_memcmp(row->secret_hash.data(), hashed.data(), hashed.size()) != 0)
    {
        return std::nullopt;
    }

    if (!row->last_used_at.has_value() || now - *row->last_used_at >= kTouchInterval.count())
    {
        const auto expires_at = row->kind == "session"
            ? std::optional(now + kSessionIdleTimeout.count())
            : std::nullopt;


        // do not let a failed Touch stop token verification

        try
        {
            TokenRows::Touch(db, row->id, now, expires_at);
            row->last_used_at = now;

            if (expires_at)
            {
                row->expires_at = expires_at;
            }
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to update last use of token " << row->id << ": " << e.what();
        }
    }

    return row;
}
