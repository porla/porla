#include "password.hpp"

#include <sodium.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>

using porla::Auth::Password;

namespace
{
    struct Zero
    {
        std::string value;
        ~Zero() { if (!value.empty()) sodium_memzero(value.data(), value.size()); }
    };
}

boost::asio::awaitable<std::optional<std::string>> Password::Hash(
    boost::asio::thread_pool& pool,
    std::string               password)
{
    co_return co_await boost::asio::co_spawn(
        pool,
        [pw = Zero{std::move(password)}]() mutable -> boost::asio::awaitable<std::optional<std::string>>
        {
            std::string buf(crypto_pwhash_STRBYTES, '\0');

            const int rc = crypto_pwhash_str(
                buf.data(),
                pw.value.data(),
                pw.value.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE);

            if (rc != 0)
            {
                co_return std::nullopt;
            }

            buf.resize(std::strlen(buf.c_str()));

            co_return buf;
        },
        boost::asio::use_awaitable);
}

boost::asio::awaitable<bool> Password::Verify(
    boost::asio::thread_pool& pool,
    std::string               hashed,
    std::string               password)
{
    co_return co_await boost::asio::co_spawn(
        pool,
        [hashed = std::move(hashed), password = Zero{std::move(password)}]() mutable -> boost::asio::awaitable<bool>
        {
            co_return crypto_pwhash_str_verify(
                hashed.c_str(),
                password.value.data(),
                password.value.size()) == 0;
        },
        boost::asio::use_awaitable);
}
