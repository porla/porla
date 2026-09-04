#include "../types.hpp"

#include "../../sessions.hpp"

using porla::Lua::Types::Session;

void Session::Register(sol::state& lua)
{
    auto session = lua.new_usertype<porla::Sessions::SessionState>(
        "porla.Session",
        sol::no_constructor,
        "is_dht_running", sol::property([](porla::Sessions::SessionState& s) { return s.session->is_dht_running(); }),
        "is_listening", sol::property([](porla::Sessions::SessionState& s) { return s.session->is_listening(); }),
        "is_paused", sol::property([](porla::Sessions::SessionState& s) { return s.session->is_paused(); }),
        "name", sol::readonly(&porla::Sessions::SessionState::name),
        "pause", [](porla::Sessions::SessionState& s) { s.session->pause(); },
        "resume", [](porla::Sessions::SessionState& s) { s.session->resume(); },
        "settings", sol::overload(
            [](porla::Sessions::SessionState& state) { return state.session->get_settings(); },
            [](porla::Sessions::SessionState& state, lt::settings_pack& sp) { state.session->apply_settings(sp); })
        );
}
