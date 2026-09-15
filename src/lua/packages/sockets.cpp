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

        return std::make_shared<Types::PoTcpClient>(state);
    });

    return tbl;
}
