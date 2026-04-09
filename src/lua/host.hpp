#pragma once

#include <boost/asio.hpp>
#include <cmrc/cmrc.hpp>
#include <sol/sol.hpp>

namespace porla::Lua
{
    class Host
    {
    public:
        explicit Host(boost::asio::io_context& io);
        ~Host();

        void Run(const cmrc::embedded_filesystem& fs);

    private:
        boost::asio::io_context& m_io;
        sol::state m_lua;
    };
}