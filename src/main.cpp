#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cmrc/cmrc.hpp>
#include <curl/curl.h>
#include <uWebSockets/App.h>

#include "cmdargs.hpp"
#include "curlmulti.hpp"
#include "logger.hpp"
#include "lua/host.hpp"

int main(int argc, char* argv[])
{
    const boost::program_options::variables_map cmd = porla::CmdArgs::Parse(argc, argv);

    if (cmd.count("help"))
    {
        return porla::CmdArgs::Help();
    }

    porla::Logger::Setup(cmd);

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "curl_global_init failed";
        return 1;
    }

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGINT, SIGTERM);

    uWS::Loop::get(&io);

    porla::CurlMulti curl_multi_instance(io);

    {
        porla::Lua::Host lua_host(io, curl_multi_instance);
        lua_host.Run(cmd);

        signals.async_wait(
            [&io, &lua_host, &signals](boost::system::error_code const& ec, int signal)
            {
                BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping... Press Ctrl+C again to force";

                lua_host.Stop(5000, [&io]()
                {
                    io.stop();
                });

                signals.async_wait(
                    [&io](boost::system::error_code const& ec, int signal)
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Second interrupt received (" << signal << ") - forcing shutdown";
                        io.stop();
                    });
            });

        io.run();
    }

    curl_global_cleanup();

    BOOST_LOG_TRIVIAL(info) << "Bye";

    return 0;
}
