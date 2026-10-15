#pragma once

#include <boost/asio/io_context.hpp>

namespace porla
{
    class KeyValue;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Kv
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, KeyValue& kv);
}
