#include "../types.hpp"

#include <libtorrent/alert_types.hpp>
#include <libtorrent/session.hpp>

#include "../registry.hpp"

using porla::Lua::Types::LtSession;

void LtSession::Register(sol::state& lua)
{
    sol::table lt = lua["lt"].valid()
        ? lua["lt"].get<sol::table>()
        : lua.create_named_table("lt");

    lt["session"] = lua.new_usertype<lt::session>(
        "lt.session",
        sol::call_constructor, sol::factories([]()
        {
            return std::make_shared<lt::session>();
        }),
        "set_alert_notify", [](sol::this_state ts, std::shared_ptr<lt::session> session, sol::protected_function callback)
        {
            sol::state_view lua(ts);

            auto cb = std::make_shared<sol::protected_function>(std::move(callback));
            auto io = lua.registry()["io"].get<porla::Lua::Registry::BoostIoContext>().io;

            session->set_alert_notify([session, io, cb]()
            {
                boost::asio::post(*io, [session, cb]()
                {
                    std::vector<lt::alert*> alerts;
                    session->pop_alerts(&alerts);

                    for (const auto alert : alerts)
                    {
                        (*cb)(alert->message());
                    }
                });
            });
        }
    );
}
