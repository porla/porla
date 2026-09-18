#include "crypto.hpp"

#include <sodium.h>

#include "../pluginstate.hpp"
#include "../types/sohashstate.hpp"

using porla::Lua::Packages::Crypto;

namespace
{
    constexpr unsigned char kSecretboxV1     = 0x01;
    constexpr std::size_t kSecretboxOverhead = 1 + crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;
}

sol::object Crypto::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("auth", [](const std::string& input, const std::string& key)
    {
        if (key.size() != crypto_auth_KEYBYTES)
        {
            throw sol::error("invalid key size");
        }

        unsigned char mac[crypto_auth_BYTES];

        int result = crypto_auth(
            mac,
            reinterpret_cast<const unsigned char*>(input.c_str()),
            input.size(),
            reinterpret_cast<const unsigned char*>(key.c_str()));

        if (result != 0)
        {
            throw sol::error("crypto_auth failed");
        }

        return std::string(reinterpret_cast<const char*>(mac), sizeof(mac));
    });

    tbl.set_function("auth_keygen", []()
    {
        unsigned char key[crypto_auth_KEYBYTES];
        crypto_auth_keygen(key);

        std::string out(reinterpret_cast<const char*>(key), sizeof(key));

        sodium_memzero(key, sizeof(key));

        return out;
    });

    tbl.set_function("auth_verify", [](const std::string& mac, const std::string& input, const std::string& key)
    {
        if (mac.size() != crypto_auth_BYTES)    { throw sol::error("invalid mac size"); }
        if (key.size() != crypto_auth_KEYBYTES) { throw sol::error("invalid key size"); }

        return crypto_auth_verify(
            reinterpret_cast<const unsigned char*>(mac.c_str()),
            reinterpret_cast<const unsigned char*>(input.c_str()),
            input.size(),
            reinterpret_cast<const unsigned char*>(key.c_str())) == 0;
    });

    tbl.set_function("hash", sol::overload(
        [](const std::string& input) -> std::optional<std::string>
        {
            std::string output;
            output.resize(crypto_generichash_BYTES, '\0');

            int result = crypto_generichash(
                reinterpret_cast<unsigned char*>(output.data()),
                output.size(),
                reinterpret_cast<const unsigned char*>(input.c_str()),
                input.size(),
                NULL,
                0);

            if (result < 0)
            {
                return std::nullopt;
            }

            return output;
        },
        [](const std::string& input, const std::string& key) -> std::optional<std::string>
        {
            std::string output;
            output.resize(crypto_generichash_BYTES, '\0');

            int result = crypto_generichash(
                reinterpret_cast<unsigned char*>(output.data()),
                output.size(),
                reinterpret_cast<const unsigned char*>(input.c_str()),
                input.size(),
                reinterpret_cast<const unsigned char*>(key.c_str()),
                key.size());

            if (result < 0)
            {
                return std::nullopt;
            }

            return output;
        }
    ));

    tbl.set_function("hash_init", [](const std::string& key) -> std::shared_ptr<crypto_generichash_state>
    {
        auto state = std::make_shared<crypto_generichash_state>();

        int result = crypto_generichash_init(
            state.get(),
            reinterpret_cast<const unsigned char*>(key.c_str()),
            key.size(),
            crypto_generichash_BYTES);

        if (result < 0)
        {
            return nullptr;
        }

        return state;
    });

    tbl.set_function("memcmp", [](const std::string& left, const std::string& right)
    {
        if (left.size() != right.size()) { return false; }
        if (left.empty())                { return true;  }

        return sodium_memcmp(left.c_str(), right.c_str(), left.size()) == 0;
    });

    tbl.set_function("pwhash", [](sol::this_state ts, const std::string& input, sol::main_protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        const auto callback_id = state->RegisterCallback(callback, true);

        boost::asio::post(
            state->sodium_hash_pool,
            [weak, work = boost::asio::make_work_guard(state->IoExecutor()), callback_id, password = std::string(input)]() mutable
            {
                std::string password_hashed;
                password_hashed.resize(crypto_pwhash_STRBYTES);

                int result = crypto_pwhash_str(
                    password_hashed.data(),
                    password.c_str(),
                    password.size(),
                    crypto_pwhash_OPSLIMIT_INTERACTIVE,
                    crypto_pwhash_MEMLIMIT_INTERACTIVE);

                boost::asio::post(
                    work.get_executor(),
                    [weak, callback_id, password_hashed]()
                    {
                        auto state = weak.lock();
                        state->InvokeCallback(callback_id, sol::nil, password_hashed);
                    });

                work.reset();
            });
    });

    tbl.set_function("pwhash_verify", [](sol::this_state ts, const std::string& hash, const std::string& input, sol::main_protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        const auto callback_id = state->RegisterCallback(callback, true);

        boost::asio::post(
            state->sodium_hash_pool,
            [weak, work = boost::asio::make_work_guard(state->IoExecutor()), callback_id, hash = std::string(hash), password = std::string(input)]() mutable
            {
                int result = crypto_pwhash_str_verify(
                    hash.c_str(),
                    password.c_str(),
                    password.size());

                boost::asio::post(
                    work.get_executor(),
                    [weak, callback_id, result]()
                    {
                        auto state = weak.lock();
                        state->InvokeCallback(callback_id, sol::nil, result == 0);
                    });

                work.reset();
            });
    });

    tbl.set_function("randombytes", [](lua_Integer size)
    {
        if (size < 0)    { throw sol::error("size must be non-negative"); }
        if (size > 4096) { throw sol::error("size exceeds maximum of 4096 bytes"); }

        std::string bytes;
        bytes.resize(static_cast<std::size_t>(size));

        if (size > 0)
        {
            randombytes_buf(bytes.data(), bytes.size());
        }

        return bytes;
    });

    tbl.set_function("randombytes_uniform", [](lua_Integer upper_bound)
    {
        if (upper_bound <= 0)           { throw sol::error("upper_bound must be positive"); }
        if (upper_bound > 0xFFFFFFFFll) { throw sol::error("upper_bound too large"); }

        return static_cast<lua_Integer>(
            randombytes_uniform(static_cast<uint32_t>(upper_bound)));
    });

    tbl.set_function("secretbox", [](const std::string& message, const std::string& key)
    {
        if (key.size() != crypto_secretbox_KEYBYTES)
        {
            throw sol::error("invalid key size");
        }

        std::string out;
        out.resize(kSecretboxOverhead + message.size());

        auto* buf = reinterpret_cast<unsigned char*>(out.data());
        buf[0] = kSecretboxV1;

        randombytes_buf(buf + 1, crypto_secretbox_NONCEBYTES);

        const int result = crypto_secretbox_easy(
            buf + 1 + crypto_secretbox_NONCEBYTES,
            reinterpret_cast<const unsigned char*>(message.data()),
            message.size(),
            buf + 1,
            reinterpret_cast<const unsigned char*>(key.data()));

        if (result != 0)
        {
            throw sol::error("crypto_secretbox_easy failed");
        }

        return out;
    });

    tbl.set_function("secretbox_keygen", []()
    {
        unsigned char key[crypto_secretbox_KEYBYTES];
        crypto_secretbox_keygen(key);

        std::string out(reinterpret_cast<const char*>(key), sizeof(key));

        sodium_memzero(key, sizeof(key));

        return out;
    });

    tbl.set_function("secretbox_open", [](const std::string& box, const std::string& key) -> sol::optional<std::string>
    {
        if (key.size() != crypto_secretbox_KEYBYTES)            { throw sol::error("invalid key size"); }
        if (box.size() < kSecretboxOverhead)                    { return sol::nullopt; }
        if (static_cast<unsigned char>(box[0]) != kSecretboxV1) { return sol::nullopt; }

        const auto* buf = reinterpret_cast<const unsigned char*>(box.data());

        std::string out;
        out.resize(box.size() - kSecretboxOverhead);

        const int result = crypto_secretbox_open_easy(
            reinterpret_cast<unsigned char*>(out.data()),
            buf + 1 + crypto_secretbox_NONCEBYTES,
            box.size() - 1 - crypto_secretbox_NONCEBYTES,
            buf + 1,
            reinterpret_cast<const unsigned char*>(key.data()));

        if (result != 0)
        {
            return sol::nullopt;
        }

        return out;
    });

    return tbl;
}
