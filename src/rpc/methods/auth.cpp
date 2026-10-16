#include "auth.hpp"

#include <boost/asio/awaitable.hpp>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>
#include <sodium.h>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../auth/context.hpp"
#include "../../auth/password.hpp"
#include "../../data/models/apikeys.hpp"
#include "../../data/models/users.hpp"
#include "../../json/utils.hpp"

using boost::asio::awaitable;

namespace porla::Rpc::Methods::Auth
{
    namespace
    {
        struct InitReq
        {
            std::string username;
            std::string password;
        };

        struct KeysCreateReq
        {
            std::string                 name;
            std::optional<std::int64_t> expires_at;
        };

        struct KeysListReq {};

        struct KeysListRes
        {
            struct Key
            {
                std::string                 id;
                std::string                 name;
                std::int64_t                created_at;
                std::optional<std::int64_t> expires_at;
            };

            std::vector<Key> keys;
        };

        struct KeysRemoveReq
        {
            std::string id;
        };

        struct LoginReq
        {
            std::string username;
            std::string password;
        };

        PORLA_JSON_REQUEST(InitReq, username, password)
        PORLA_JSON_REQUEST(KeysCreateReq, name, expires_at)
        PORLA_JSON_NO_PARAMS(KeysListReq)
        PORLA_JSON_REQUEST(KeysRemoveReq, id)
        PORLA_JSON_REQUEST(LoginReq, username, password)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(KeysListRes::Key, id, name, created_at, expires_at)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(KeysListRes, keys)

        constexpr auto kDummyHash = "$argon2id$v=19$m=65536,t=2,p=1$Po8JYTODUsewD1r3zg1XsQ$icZxLXBcZRDPKybU+9MkOIdrATTmhENNG7EfckW5zvg";

        std::string CreateAuthCookie(const std::string& name, const std::string& token, int max_age_seconds, bool secure)
        {
            std::stringstream ss;
            ss << name << "=" << token << "; Path=/; HttpOnly; Max-Age=" << max_age_seconds << "; SameSite=Strict";

            if (secure)
            {
                ss << "; Secure";
            }

            return ss.str();
        }

        awaitable<void> Init(boost::asio::thread_pool& hash_pool, sqlite3* db, const InitReq& req)
        {
            if (porla::Data::Models::Users::Any(db))
            {
                throw Error(-1, "Already initialized");
            }

            const auto hashed_password = co_await porla::Auth::Password::Hash(
                hash_pool,
                std::move(req.password));

            if (!hashed_password.has_value())
            {
                throw Error(-2, "Failed to hash password");
            }

            if (porla::Data::Models::Users::Any(db))
            {
                BOOST_LOG_TRIVIAL(warning) << "An initial user was created while hashing password";
                throw Error(-1, "Already initialized");
            }

            porla::Data::Models::Users::Insert(
                db,
                porla::Data::Models::Users::User{
                    .username        = req.username,
                    .password_hashed = hashed_password.value()
                });

            BOOST_LOG_TRIVIAL(info) << "User " << req.username << " created";
        }

        awaitable<nlohmann::json> Login(boost::asio::thread_pool& hash_pool, sqlite3* db, const std::string& secret_key, const LoginReq req, Call& call)
        {
            const auto user = porla::Data::Models::Users::GetByUsername(db, req.username);

            const std::string stored = user.has_value()
                ? user->password_hashed
                : kDummyHash;

            const auto [ verified, needs_rehash ] = co_await porla::Auth::Password::Verify(
                hash_pool,
                stored,
                req.password);

            if (!verified || !user.has_value())
            {
                throw Error(-1, "Invalid username/password combination");
            }

            if (needs_rehash)
            {
                const auto fresh_password = co_await porla::Auth::Password::Hash(
                    hash_pool,
                    req.password);

                if (fresh_password)
                {
                    Data::Models::Users::UpdatePassword(
                        db,
                        user->id,
                        fresh_password.value());

                    BOOST_LOG_TRIVIAL(info) << "Updated password hash for user " << user->username;
                }
            }

            const auto token = jwt::create()
                .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{1})
                .set_issuer("porla")
                .set_issued_at(std::chrono::system_clock::now())
                .set_subject(user->username)
                .set_type("JWS")
                .set_payload_claim("scope", "*")
                .sign(jwt::algorithm::hs256(secret_key));

            call.Header("Set-Cookie", CreateAuthCookie("porla-auth-token", token, 86400, false));

            co_return nlohmann::json({
                {"token", token}
            });
        }

        awaitable<nlohmann::json> KeysCreate(sqlite3* db, const KeysCreateReq& req)
        {
            if (req.name.empty() || req.name.size() > 1024)
            {
                throw Error(-1, "Invalid name - length must be between 1-1024");
            }

            if (req.expires_at.has_value() && req.expires_at.value() <= std::time(nullptr))
            {
                throw Error(-2, "Invalid expires_at - must be in the future");
            }

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

            Data::Models::ApiKeys::Insert(
                db,
                key_id,
                req.name,
                std::vector<char>(
                    reinterpret_cast<char*>(secret_hashed),
                    reinterpret_cast<char*>(secret_hashed) + sizeof(secret_hashed)),
                req.expires_at);

            BOOST_LOG_TRIVIAL(info) << "API key " << req.name << " created with ID " << key_id;

            co_return nlohmann::json({
                {"id", key_id},
                {"key", "porla_" + std::string(key_id) + "_" + secret_encoded}
            });
        }

        awaitable<nlohmann::json> KeysList(sqlite3* db, const KeysListReq& req)
        {
            const auto keys = Data::Models::ApiKeys::List(db);

            std::vector<KeysListRes::Key> output_keys;
            output_keys.reserve(keys.size());

            std::transform(
                keys.begin(),
                keys.end(),
                std::back_inserter(output_keys),
                [](const Data::Models::ApiKeys::ApiKey& model)
                {
                    return KeysListRes::Key{
                        .id         = model.id,
                        .name       = model.name,
                        .created_at = model.created_at,
                        .expires_at = model.expires_at
                    };
                });

            co_return nlohmann::json({
                {"keys", output_keys}
            });
        }

        awaitable<void> KeysRemove(sqlite3* db, const KeysRemoveReq& req)
        {
            Data::Models::ApiKeys::Remove(db, req.id);
            BOOST_LOG_TRIVIAL(info) << "API key " << req.id << " removed";
            co_return;
        }
    }

    void Register(
        boost::asio::io_context& io,
        JsonRpc& jsonrpc,
        boost::asio::thread_pool& hash_pool,
        sqlite3* db,
        const std::string& secret_key)
    {
        using Kind = porla::Auth::Context::Kind;

        const auto ex = io.get_executor();

        jsonrpc.Register("auth.init",  { Kind::Anon, Kind::Machine, Kind::User }, MakeMethod<InitReq>(ex, std::bind_front(Init, std::ref(hash_pool), db)));
        jsonrpc.Register("auth.login", { Kind::Anon, Kind::Machine, Kind::User }, MakeMethod<LoginReq>(ex, std::bind_front(Login, std::ref(hash_pool), db, std::cref(secret_key))));

        jsonrpc.Register("auth.keys.create", { Kind::User }, MakeMethod<KeysCreateReq>(ex, std::bind_front(KeysCreate, db)));
        jsonrpc.Register("auth.keys.list",   { Kind::User }, MakeMethod<KeysListReq>(ex, std::bind_front(KeysList, db)));
        jsonrpc.Register("auth.keys.remove", { Kind::User }, MakeMethod<KeysRemoveReq>(ex, std::bind_front(KeysRemove, db)));
    }
}
