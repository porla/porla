#include "sockets.hpp"

#include <memory>

#include <boost/asio/ssl.hpp>

#include "../types/potcpclient.hpp"

using porla::Lua::Packages::Sockets;

sol::object Sockets::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("tcp_client", [](sol::this_state ts, std::optional<sol::table> options) -> std::shared_ptr<Types::PoTcpClient>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<boost::asio::ssl::context> tls_ctx;

        if (options.has_value() && (*options)["tls"].valid())
        {
            boost::system::error_code ec;

            tls_ctx = std::make_shared<boost::asio::ssl::context>(
                boost::asio::ssl::context::tls_client);

            tls_ctx->set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::no_tlsv1 |
                boost::asio::ssl::context::no_tlsv1_1,
                ec);

            tls_ctx->set_default_verify_paths(ec);
        }

        return std::make_shared<Types::PoTcpClient>(state, tls_ctx);
    });

    return tbl;
}
