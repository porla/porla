#pragma once

#include <sol/sol.hpp>

namespace porla
{
    class ISession;
}

namespace porla::Lua::UserTypes
{
    class Session
    {
    public:
        static void Register(sol::state& lua);

        explicit Session(ISession& session);

    private:
        ISession& m_session;
    };
}
