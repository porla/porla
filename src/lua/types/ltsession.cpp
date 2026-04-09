#include "../types.hpp"

#include <libtorrent/alert_types.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
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
        sol::call_constructor, sol::factories(
            []()
            {
                return std::make_shared<lt::session>();
            },
            [](const lt::settings_pack& settings)
            {
                return std::make_shared<lt::session>(settings);
            }
        ),
        "add_extension",    [](std::shared_ptr<lt::session> session, const std::string& name)
        {
            if (name == "smart_ban")   { session->add_extension(&lt::create_smart_ban_plugin); }
            if (name == "ut_metadata") { session->add_extension(&lt::create_ut_metadata_plugin); }
            if (name == "ut_pex")      { session->add_extension(&lt::create_ut_pex_plugin); }
        },
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
