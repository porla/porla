#pragma once

#include <boost/asio/io_context.hpp>

namespace porla
{
    class Sessions;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Sessions
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Sessions& sessions);
}
