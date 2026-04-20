#include "host.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>
#include <cmrc/cmrc.hpp>

#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

#include "../curlmulti.hpp"

CMRC_DECLARE(porla_lua_bootstrap);

namespace fs = std::filesystem;
using porla::Lua::Host;

Host::Host(boost::asio::io_context& io, porla::CurlMulti& cm)
    : m_cm(cm)
    , m_io(io)
    , m_active_coroutines_cleanup_timer(io)
    , m_shutdown_deadline_timer(io)
    , m_shutdown_poll_timer(io)
{
    ScheduleCoroutineCleanup();
}

Host::~Host() = default;

void Host::Run(const boost::program_options::variables_map& args)
{
    m_lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    m_lua.registry()["io"]   = porla::Lua::Registry::BoostIoContext{.io = &m_io};
    m_lua.registry()["curl"] = &m_cm;

    m_lua["args"] = m_lua.create_table();
    if (args.count("config-file"))            m_lua["args"]["config-file"]            = args["config-file"].as<std::string>();
    if (args.count("core-dir"))               m_lua["args"]["core-dir"]               = args["core-dir"].as<std::string>();
    if (args.count("core-zip"))               m_lua["args"]["core-zip"]               = args["core-zip"].as<std::string>();
    if (args.count("core-github-repository")) m_lua["args"]["core-github-repository"] = args["core-github-repository"].as<std::string>();
    if (args.count("core-github-release"))    m_lua["args"]["core-github-release"]    = args["core-github-release"].as<std::string>();
    if (args.count("state-dir"))              m_lua["args"]["state-dir"]              = args["state-dir"].as<std::string>();

    m_lua["print"] = [](sol::this_state ts, const sol::variadic_args& args)
    {
        sol::state_view lua(ts);
        std::ostringstream oss;

        for (size_t i = 0; i < args.size(); ++i)
        {
            if (i > 0) oss << "\t";
            oss << lua["tostring"](args[i]).get<std::string>();
        }

        BOOST_LOG_TRIVIAL(info) << oss.str();
    };

    porla::Lua::Types::Fs::Register(m_lua);
    porla::Lua::Types::HttpClient::Register(m_lua);
    porla::Lua::Types::HttpServer::Register(m_lua);
    porla::Lua::Types::Json::Register(m_lua);
    porla::Lua::Types::Libzip::Register(m_lua);
    porla::Lua::Types::Log::Register(m_lua);
    porla::Lua::Types::LtAddTorrentParams::Register(m_lua);
    porla::Lua::Types::LtAlert::Register(m_lua);
    porla::Lua::Types::LtAnnounceEndpoint::Register(m_lua);
    porla::Lua::Types::LtAnnounceEntry::Register(m_lua);
    porla::Lua::Types::LtAnnounceInfohash::Register(m_lua);
    porla::Lua::Types::LtDownloadPriority::Register(m_lua);
    porla::Lua::Types::LtInfoHash::Register(m_lua);
    porla::Lua::Types::LtPeerInfo::Register(m_lua);
    porla::Lua::Types::LtSession::Register(m_lua);
    porla::Lua::Types::LtSettingsPack::Register(m_lua);
    porla::Lua::Types::LtStorageMode::Register(m_lua);
    porla::Lua::Types::LtTorrentFlags::Register(m_lua);
    porla::Lua::Types::LtTorrentHandle::Register(m_lua);
    porla::Lua::Types::LtTorrentInfo::Register(m_lua);
    porla::Lua::Types::LtTorrentStatus::Register(m_lua);
    porla::Lua::Types::Mmdb::Register(m_lua);
    porla::Lua::Types::Timer::Register(m_lua);
    porla::Lua::Types::Toml::Register(m_lua);

    porla::Lua::Packages::Sqlite::Register(m_lua);

    if (args.count("bootstrap-file"))
    {
        const auto bootstrap_file = fs::absolute(args["bootstrap-file"].as<std::string>());

        BOOST_LOG_TRIVIAL(info) << "Executing external bootstrap file " << bootstrap_file;
        m_bootstrap_table = m_lua.script_file(bootstrap_file);
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Executing embedded bootstrap.lua file";

        const auto bootstrap_fs  = cmrc::porla_lua_bootstrap::get_filesystem();
        const auto bootstrap_lua = bootstrap_fs.open("bootstrap.lua");

        m_bootstrap_table = m_lua.script(
            std::string(bootstrap_lua.begin(), bootstrap_lua.end()),
            "bootstrap.lua");
    }

    if (!m_bootstrap_table.valid())
    {
        BOOST_LOG_TRIVIAL(error) << "No valid bootstrap table returned from script";
        return;
    }

    sol::function load_fn = m_bootstrap_table["load"];
    sol::thread th = sol::thread::create(m_lua);

    sol::coroutine(th.state(), load_fn)();

    m_active_coroutines.push_back(std::move(th));
}

void Host::Stop(int timeout_ms, std::function<void()> callback)
{
    m_active_coroutines_cleanup_timer.cancel();

    sol::optional<sol::function> unload_fn = m_bootstrap_table["unload"];

    if (!unload_fn)
    {
        boost::asio::post(m_io, callback);
        return;
    }

    sol::thread th = sol::thread::create(m_lua);
    lua_State* L = th.state();

    auto result = sol::coroutine(L, *unload_fn)();

    if (!result.valid())
    {
        sol::error err = result;
        BOOST_LOG_TRIVIAL(error) << "Error when unloading bootstrap program: " << err.what();
        boost::asio::post(m_io, callback);
        return;
    }

    m_active_coroutines.push_back(std::move(th));

    if (lua_status(L) != LUA_YIELD)
    {
        boost::asio::post(m_io, callback);
        return;
    }

    // Hard deadline
    m_shutdown_deadline_timer.expires_after(std::chrono::milliseconds(timeout_ms));
    m_shutdown_deadline_timer.async_wait([this, callback](boost::system::error_code ec)
    {
        if (ec)
        {
            return;
        }

        BOOST_LOG_TRIVIAL(warning) << "Lua host shutdown timed out";

        m_shutdown_poll_timer.cancel();

        boost::asio::post(m_io, callback);
    });

    PollShutdown(L, callback);
}

void Host::PollShutdown(lua_State* L, std::function<void()> callback)
{
    m_shutdown_poll_timer.expires_after(std::chrono::milliseconds(50));
    m_shutdown_poll_timer.async_wait([this, L, callback](boost::system::error_code ec)
    {
        if (ec)
        {
            return;
        }

        if (lua_status(L) != LUA_YIELD)
        {
            m_shutdown_deadline_timer.cancel();
            boost::asio::post(m_io, callback);
        }
        else
        {
            PollShutdown(L, callback);
        }
    });
}

void Host::ScheduleCoroutineCleanup()
{
    m_active_coroutines_cleanup_timer.expires_after(std::chrono::seconds(10));

    m_active_coroutines_cleanup_timer.async_wait([this](boost::system::error_code ec)
    {
        if (ec)
        {
            return;
        }

        auto before = m_active_coroutines.size();

        m_active_coroutines.erase(
            std::remove_if(
                m_active_coroutines.begin(),
                m_active_coroutines.end(),
                [](const sol::thread& th)
                {
                    return lua_status(th.state()) != LUA_YIELD;
                }),
            m_active_coroutines.end());

        auto removed = before - m_active_coroutines.size();

        if (removed > 0)
        {
            BOOST_LOG_TRIVIAL(debug)
                << "Cleaned up " << removed << " finished coroutines, "
                << m_active_coroutines.size() << " active";
        }

        ScheduleCoroutineCleanup();
    });
}
