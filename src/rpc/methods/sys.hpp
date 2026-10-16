#pragma once

#include <boost/asio/io_context.hpp>
#include <sqlite3.h>

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Sys
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, sqlite3* db);
}
