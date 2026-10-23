#include <boost/asio.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <cmrc/cmrc.hpp>
#include <curl/curl.h>
#include <sodium.h>

#include "cmdargs.hpp"
#include "config.hpp"
#include "curlmulti.hpp"
#include "events.hpp"
#include "keyvalue.hpp"
#include "logger.hpp"
#include "lua/pluginsource.hpp"
#include "plugins.hpp"
#include "presets.hpp"
#include "sessions/session.hpp"
#include "sessions/sessions.hpp"
#include "sessions/torrent.hpp"
#include "timer.hpp"
#include "torrents.hpp"

#include "auth/authenticator.hpp"

#include "rpc/jsonrpc.hpp"
#include "rpc/methods/auth.hpp"
#include "rpc/methods/fs.hpp"
#include "rpc/methods/kv.hpp"
#include "rpc/methods/mmdb.hpp"
#include "rpc/methods/plugins.hpp"
#include "rpc/methods/presets.hpp"
#include "rpc/methods/sessions.hpp"
#include "rpc/methods/sys.hpp"
#include "rpc/methods/torrents.hpp"

CMRC_DECLARE(porla_lua);

extern "C"
{
    // normally called by us_loop_run which we don't use since we run the asio loop ourselves
    void us_internal_free_closed_sockets(struct us_loop_t* loop);
}

static void Traverse(
    const cmrc::embedded_filesystem& fs,
    const std::string& dir,
    std::map<std::string, std::vector<char>>& entries)
{
    for (auto&& entry : fs.iterate_directory(dir))
    {
        const auto path = dir.empty()
            ? entry.filename()
            : dir + "/" + entry.filename();

        if (entry.is_directory())
        {
            Traverse(fs, path, entries);
        }
        else
        {
            const auto file = fs.open(path);
            entries.emplace(path, std::vector<char>(file.begin(), file.end()));
        }
    }
}

namespace M = porla::Rpc::Methods;

int main(int argc, char* argv[])
{
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
    {
        std::cerr << "curl_global_init failed";
        return -1;
    }

    if (sodium_init() == -1)
    {
        std::cerr << "sodium_init failed";
        return -1;
    }

    boost::program_options::variables_map cmd;

    try
    {
        cmd = porla::CmdArgs::Parse(argc, argv);
    }
    catch(const boost::program_options::error& ex)
    {
        std::cerr
            << ex.what()
            << " (run porla --help to see available options)"
            << std::endl;

        return 1;
    }

    if (cmd.count("help"))
    {
        return porla::CmdArgs::Help();
    }

    porla::Logger::Setup(cmd);

    std::unique_ptr<porla::Config> cfg;

    try
    {
        cfg = porla::Config::Load(cmd);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(fatal) << "Failed to load configuration: " << ex.what();
        return -1;
    }

    boost::asio::io_context  io;
    boost::asio::thread_pool sodium_hash_pool(2);

    {
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);

        signals.async_wait(
            [&io](boost::system::error_code const& ec, int signal)
            {
                BOOST_LOG_TRIVIAL(info) << "Interrupt received (" << signal << ") - stopping...";
                io.stop();
            });

        auto* http_loop = reinterpret_cast<us_loop_t*>(uWS::Loop::get(&io));
        uWS::App http_server;

        porla::Events events(io);

        porla::KeyValue kv(porla::KeyValueOptions{
            .db     = cfg->db,
            .events = events
        });

        porla::Presets presets(porla::PresetsOptions{
            .db     = cfg->db,
            .events = events
        });

        auto authenticator       = std::make_shared<porla::Auth::Authenticator>(cfg->db);
        auto curl_multi_instance = porla::CurlMulti::Create(io);
        auto jsonrpc             = porla::Rpc::JsonRpc::Create(authenticator);

        porla::Sessions sessions(porla::SessionsOptions{
            .db     = cfg->db,
            .events = events,
            .io     = io
        });

        porla::Torrents torrents(porla::TorrentsOptions{
            .db       = cfg->db,
            .events   = events,
            .presets  = presets,
            .sessions = sessions
        });

        porla::Plugins plugins(porla::PluginsOptions{
            .cfg         = *cfg,
            .curl_multi  = curl_multi_instance,
            .db          = cfg->db,
            .events      = events,
            .hash_pool   = sodium_hash_pool,
            .http_server = &http_server,
            .jsonrpc     = jsonrpc,
            .io          = io,
            .kv          = kv,
            .presets     = presets,
            .sessions    = sessions,
            .torrents    = torrents
        });

        porla::Rpc::Methods::Auth::Register(io, *jsonrpc, sodium_hash_pool, cfg->db);
        porla::Rpc::Methods::Fs::Register(io, *jsonrpc);
        porla::Rpc::Methods::Kv::Register(io, *jsonrpc, kv);
        porla::Rpc::Methods::Mmdb::Register(io, *jsonrpc, kv, events);
        porla::Rpc::Methods::Plugins::Register(io, *curl_multi_instance, *jsonrpc, plugins, cfg->state_dir);
        porla::Rpc::Methods::Presets::Register(io, *jsonrpc, presets);
        porla::Rpc::Methods::Sessions::Register(io, *jsonrpc, sessions);
        porla::Rpc::Methods::Sys::Register(io, *jsonrpc, cfg->db);
        porla::Rpc::Methods::Torrents::Register(io, *jsonrpc, events, sessions, torrents);

              auto embedded_core       = porla::Lua::PluginSource{.entrypoint = "plugin.lua", .sources = {}};
        const auto embedded_core_files = cmrc::porla_lua::get_filesystem();

        Traverse(embedded_core_files, "", embedded_core.sources);

        if (embedded_core.sources.find(embedded_core.entrypoint) != embedded_core.sources.end())
        {
            plugins.SetCore(embedded_core);
        }

        sessions.Load(
            [&cfg, &http_server, &jsonrpc, &plugins]()
            {
                try
                {
                    plugins.LoadAll();
                }
                catch (const std::exception& e)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to load plugins: " << e.what();
                }

                std::string http_base_path = cfg->http_base_path.value_or("/");
                if (http_base_path.empty())        http_base_path = "/";
                if (http_base_path[0] != '/')      http_base_path = "/" + http_base_path;
                if (http_base_path.ends_with("/")) http_base_path = http_base_path.substr(0, http_base_path.size() - 1);

                http_server.post(http_base_path + "/api/v1/jsonrpc", jsonrpc->HttpHandler());

                http_server.listen(
                    cfg->http_host.value_or("127.0.0.1"),
                    cfg->http_port.value_or(1337),
                    [](const auto* t)
                    {
                        BOOST_LOG_TRIVIAL(info) << "HTTP server listening";
                    });
            });

        const auto sweep_timer = porla::Timer::Interval(
            io,
            std::chrono::seconds(1),
            [http_loop]() { us_internal_free_closed_sockets(http_loop); });

        for (;;)
        {
            try
            {
                io.run();
                break;
            }
            catch(const std::exception& e)
            {
                BOOST_LOG_TRIVIAL(error) << "Unhandled exception in event loop: " << e.what();
            }
        }
    }

    curl_global_cleanup();

    BOOST_LOG_TRIVIAL(info) << "Bye";

    return 0;
}
