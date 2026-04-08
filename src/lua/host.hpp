#pragma once

#include <cmrc/cmrc.hpp>
#include <sol/sol.hpp>

namespace porla::Lua
{
    class Host
    {
    public:
        void Run(const cmrc::embedded_filesystem& fs);

    private:
        sol::state m_lua;
    };
}