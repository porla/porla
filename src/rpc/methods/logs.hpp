#pragma once

#include <boost/asio/io_context.hpp>

namespace porla::Log
{
    class Buffer;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Logs
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, const Log::Buffer& logs);
}
