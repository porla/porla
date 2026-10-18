#pragma once

#include <boost/asio/io_context.hpp>

namespace porla
{
    class Events;
    class Sessions;
    class Torrents;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Torrents
{
    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Events& events, porla::Sessions& sessions, porla::Torrents& torrents);
}
