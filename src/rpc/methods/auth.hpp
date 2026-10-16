#pragma once

#include <string>

#include <boost/asio/io_context.hpp>
#include <boost/asio/thread_pool.hpp>
#include <sqlite3.h>

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Auth
{
    void Register(
        boost::asio::io_context& io,
        JsonRpc& jsonrpc,
        boost::asio::thread_pool& hash_pool,
        sqlite3* db,
        const std::string& secret_key);
}
