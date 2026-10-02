#include "poauthcontext.hpp"

#include "../../auth/context.hpp"

using porla::Auth::Context;
using porla::Lua::Types::PoAuthContext;

void PoAuthContext::Register(sol::state& lua)
{
    lua.new_usertype<Context>(
        "PoAuthContext",
        sol::no_constructor,
        "is_authenticated", &Context::IsAuthenticated,
        "kind", sol::property([](const Context& c)
        {
            if (c.kind == Context::Kind::Anon)    { return "anon"; }
            if (c.kind == Context::Kind::Machine) { return "machine"; }
            if (c.kind == Context::Kind::User)    { return "user"; }
            throw sol::error("Invalid kind");
        }),
        "subject", sol::readonly(&Context::subject));
}
