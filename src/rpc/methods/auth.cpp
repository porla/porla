#include "auth.hpp"

#include <boost/asio/awaitable.hpp>
#include <sodium.h>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../auth/context.hpp"
#include "../../auth/password.hpp"
#include "../../auth/tokens.hpp"
#include "../../data/models/tokens.hpp"
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
                std::optional<std::int64_t> last_used_at;
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

        struct LogoutReq {};

        struct SessionsListReq {};

        struct SessionsListRes
        {
            struct Session
            {
                std::string                 id;
                std::int64_t                created_at;
                std::optional<std::int64_t> last_used_at;
                std::optional<std::int64_t> expires_at;
                std::optional<std::string>  user_agent;
                std::optional<std::string>  remote_address;
                bool                        current;
            };

            std::vector<Session> sessions;
        };

        struct SessionsRemoveReq
        {
            std::string id;
        };

        PORLA_JSON_REQUEST(InitReq, username, password)
        PORLA_JSON_REQUEST(KeysCreateReq, name, expires_at)
        PORLA_JSON_NO_PARAMS(KeysListReq)
        PORLA_JSON_REQUEST(KeysRemoveReq, id)
        PORLA_JSON_REQUEST(LoginReq, username, password)
        PORLA_JSON_NO_PARAMS(LogoutReq)
        PORLA_JSON_NO_PARAMS(SessionsListReq)
        PORLA_JSON_REQUEST(SessionsRemoveReq, id)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(KeysListRes::Key, id, name, created_at, expires_at, last_used_at)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(KeysListRes, keys)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(SessionsListRes::Session, id, created_at, last_used_at, expires_at, user_agent, remote_address, current)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(SessionsListRes, sessions)

        constexpr int kCookieMaxAge = 400 * 24 * 60 * 60;
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

            if (req.username.empty() || req.password.empty())
            {
                throw Error(-3, "Username and password must not be empty");
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

        awaitable<nlohmann::json> Login(boost::asio::thread_pool& hash_pool, sqlite3* db, const LoginReq& req, Call& call)
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

            const auto now = static_cast<std::int64_t>(std::time(nullptr));

            if (const int removed = Data::Models::Tokens::RemoveExpiredSessions(db, now); removed > 0)
            {
                BOOST_LOG_TRIVIAL(debug) << "Removed " << removed << " expired session(s)";
            }

            // Make room for the new session.
            if (const int removed = Data::Models::Tokens::RemoveLeastRecentlyUsedSessions(db, user->id, porla::Auth::Tokens::kMaxSessionsPerUser - 1); removed > 0)
            {
                BOOST_LOG_TRIVIAL(info)
                    << "User " << user->username << " has more than "
                    << porla::Auth::Tokens::kMaxSessionsPerUser
                    << " sessions - ended the " << removed << " least recently used";
            }

            const auto& caller = call.Caller();

            const auto session = porla::Auth::Tokens::Issue(
                db,
                Data::Models::Tokens::Token{
                    .kind           = "session",
                    .user_id        = user->id,
                    .expires_at     = now + porla::Auth::Tokens::kSessionIdleTimeout.count(),
                    .last_used_at   = now,
                    .user_agent     = caller.user_agent.empty() ? std::nullopt : std::optional(caller.user_agent),
                    .remote_address = caller.remote_address.empty() ? std::nullopt : std::optional(caller.remote_address)
                });

            BOOST_LOG_TRIVIAL(info) << "User " << user->username << " logged in (session " << session.id << ")";

            call.Header("Set-Cookie", CreateAuthCookie("porla-auth-token", session.token, kCookieMaxAge, false));

            co_return nlohmann::json({
                {"token", session.token}
            });
        }

        awaitable<void> Logout(sqlite3* db, const LogoutReq& req, Call& call)
        {
            if (const auto& id = call.Caller().token_id)
            {
                Data::Models::Tokens::Remove(db, *id);
                BOOST_LOG_TRIVIAL(info) << "Session " << *id << " logged out";
            }

            call.Header("Set-Cookie", CreateAuthCookie("porla-auth-token", "", 0, false));

            co_return;
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

            const auto key = porla::Auth::Tokens::Issue(
                db,
                Data::Models::Tokens::Token{
                    .kind       = "api_key",
                    .name       = req.name,
                    .expires_at = req.expires_at
                });

            BOOST_LOG_TRIVIAL(info) << "API key " << req.name << " created with ID " << key.id;

            co_return nlohmann::json({
                {"id", key.id},
                {"key", key.token}
            });
        }

        awaitable<nlohmann::json> KeysList(sqlite3* db, const KeysListReq& req)
        {
            const auto keys = Data::Models::Tokens::List(db, "api_key");

            std::vector<KeysListRes::Key> output_keys;
            output_keys.reserve(keys.size());

            std::transform(
                keys.begin(),
                keys.end(),
                std::back_inserter(output_keys),
                [](const Data::Models::Tokens::Token& model)
                {
                    return KeysListRes::Key{
                        .id           = model.id,
                        .name         = model.name.value_or(""),
                        .created_at   = model.created_at,
                        .expires_at   = model.expires_at,
                        .last_used_at = model.last_used_at
                    };
                });

            co_return nlohmann::json({
                {"keys", output_keys}
            });
        }

        awaitable<void> KeysRemove(sqlite3* db, const KeysRemoveReq& req)
        {
            const auto token = Data::Models::Tokens::GetById(db, req.id);

            if (!token.has_value() || token->kind != "api_key")
            {
                throw Error(-1, "API key not found");
            }

            Data::Models::Tokens::Remove(db, req.id);
            BOOST_LOG_TRIVIAL(info) << "API key " << req.id << " removed";
            co_return;
        }

        awaitable<nlohmann::json> SessionsList(sqlite3* db, const SessionsListReq& req, Call& call)
        {
            SessionsListRes res;

            for (const auto& token : Data::Models::Tokens::List(db, "session"))
            {
                res.sessions.push_back(SessionsListRes::Session{
                    .id             = token.id,
                    .created_at     = token.created_at,
                    .last_used_at   = token.last_used_at,
                    .expires_at     = token.expires_at,
                    .user_agent     = token.user_agent,
                    .remote_address = token.remote_address,
                    .current        = token.id == call.Caller().token_id
                });
            }

            co_return res;
        }

        awaitable<void> SessionsRemove(sqlite3* db, const SessionsRemoveReq& req)
        {
            const auto token = Data::Models::Tokens::GetById(db, req.id);

            if (!token.has_value() || token->kind != "session")
            {
                throw Error(-1, "Session not found");
            }

            Data::Models::Tokens::Remove(db, req.id);
            BOOST_LOG_TRIVIAL(info) << "Session " << req.id << " removed";
            co_return;
        }
    }

    void Register(
        boost::asio::io_context& io,
        JsonRpc& jsonrpc,
        boost::asio::thread_pool& hash_pool,
        sqlite3* db)
    {
        using Kind = porla::Auth::Context::Kind;

        const auto ex = io.get_executor();

        jsonrpc.Register("auth.init",  { Kind::Anon, Kind::Machine, Kind::User }, MakeMethod<InitReq>(ex, std::bind_front(Init, std::ref(hash_pool), db)));
        jsonrpc.Register("auth.login", { Kind::Anon, Kind::Machine, Kind::User }, MakeMethod<LoginReq>(ex, std::bind_front(Login, std::ref(hash_pool), db)));
        jsonrpc.Register("auth.logout", { Kind::User }, MakeMethod<LogoutReq>(ex, std::bind_front(Logout, db)));

        jsonrpc.Register("auth.keys.create", { Kind::User }, MakeMethod<KeysCreateReq>(ex, std::bind_front(KeysCreate, db)));
        jsonrpc.Register("auth.keys.list",   { Kind::User }, MakeMethod<KeysListReq>(ex, std::bind_front(KeysList, db)));
        jsonrpc.Register("auth.keys.remove", { Kind::User }, MakeMethod<KeysRemoveReq>(ex, std::bind_front(KeysRemove, db)));

        jsonrpc.Register("auth.sessions.list",   { Kind::User }, MakeMethod<SessionsListReq>(ex, std::bind_front(SessionsList, db)));
        jsonrpc.Register("auth.sessions.remove", { Kind::User }, MakeMethod<SessionsRemoveReq>(ex, std::bind_front(SessionsRemove, db)));
    }
}
