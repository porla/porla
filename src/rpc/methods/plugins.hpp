#pragma once

#include <filesystem>

#include <boost/asio/io_context.hpp>

namespace porla
{
    class CurlMulti;
    class Events;
    class Plugins;
}

namespace porla::Rpc
{
    class JsonRpc;
}

namespace porla::Rpc::Methods::Plugins
{
    void Register(
        boost::asio::io_context& io,
        CurlMulti& curl,
        JsonRpc& jsonrpc,
        porla::Plugins& plugins,
        const std::filesystem::path& state_dir);
}
