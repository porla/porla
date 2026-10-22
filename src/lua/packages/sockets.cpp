#include "../packages.hpp"

#include <memory>

#include <boost/asio/ssl.hpp>

#include "../pluginstate.hpp"
#include "../types/potcpclient.hpp"

using porla::Lua::Packages::Sockets;

sol::object Sockets::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("tcp_client", [](sol::this_state ts, std::optional<sol::table> options) -> std::shared_ptr<Types::PoTcpClient>
    {
        auto state = LuaState::From(ts);

        if (state == nullptr)
        {
            return nullptr;
        }

        auto client = std::make_shared<Types::PoTcpClient>(state);
        state->RegisterTcpClient(client);

        return client;
    });

    return tbl;
}
