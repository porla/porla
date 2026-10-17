#pragma once

#include <boost/asio/io_context.hpp>

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Fs
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc);
}
