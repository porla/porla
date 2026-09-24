#pragma once

#include <optional>
#include <string>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/thread_pool.hpp>

namespace porla::Auth
{
    struct Password
    {

        static boost::asio::awaitable<std::optional<std::string>> Hash(
            boost::asio::thread_pool& pool,
            std::string               password);
            
        static boost::asio::awaitable<bool> Verify(
            boost::asio::thread_pool& pool,
            std::string               hashed,
            std::string               password);
    };
}
