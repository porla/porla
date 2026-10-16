#pragma once

#include <boost/asio/io_context.hpp>

namespace porla
{
    class Presets;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Presets
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Presets& presets);
}
