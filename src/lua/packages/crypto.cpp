#include "crypto.hpp"

#include <sodium.h>

#include "../pluginstate.hpp"
#include "../types/sohashstate.hpp"

using porla::Lua::Packages::Crypto;

sol::object Crypto::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

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

    tbl.set_function("random_bytes", [](int n)
    {
        std::string bytes;
        bytes.resize(n, '\0');

        randombytes_buf(bytes.data(), n);

        return bytes;
    });

    tbl.set_function("secretbox", [](const std::string& message, const std::string& key)
    {
        if (key.size() != crypto_secretbox_KEYBYTES)
        {
            throw sol::error("invalid key size");
        }

        std::string out;
        out.resize(crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES + message.size());

        auto* buf = reinterpret_cast<unsigned char*>(out.data());

        randombytes_buf(buf, crypto_secretbox_NONCEBYTES);

        const int result = crypto_secretbox_easy(
            buf + crypto_secretbox_NONCEBYTES,
            reinterpret_cast<const unsigned char*>(message.data()),
            message.size(),
            buf,
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
        constexpr std::size_t overhead = crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES;

        if (key.size() != crypto_secretbox_KEYBYTES) { throw sol::error("invalid key size"); }
        if (box.size() < overhead)                   { return sol::nullopt; }

        const auto* buf = reinterpret_cast<const unsigned char*>(box.data());

        std::string out;
        out.resize(box.size() - overhead);

        const int result = crypto_secretbox_open_easy(
            reinterpret_cast<unsigned char*>(out.data()),
            buf + crypto_secretbox_NONCEBYTES,
            box.size() - crypto_secretbox_NONCEBYTES,
            buf,
            reinterpret_cast<const unsigned char*>(key.data()));

        if (result != 0)
        {
            return sol::nullopt;
        }

        return out;
    });

    return tbl;
}
