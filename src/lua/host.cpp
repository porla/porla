#include "host.hpp"

#include <boost/log/trivial.hpp>

#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

using porla::Lua::Host;

Host::Host(boost::asio::io_context& io)
    : m_io(io)
{
}

Host::~Host()
{
    m_lua["on_unload"]();
}

void Host::Run(const cmrc::embedded_filesystem& fs)
{
    m_lua.open_libraries(
        sol::lib::base,
        sol::lib::io,
        sol::lib::os,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table);

    m_lua.registry()["io"] = porla::Lua::Registry::BoostIoContext{.io = &m_io};

    m_lua["package"]["path"] = "/workspaces/porla/lua/?.lua;/workspaces/porla/lua/?/init.lua";

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
    porla::Lua::Types::Toml::Register(m_lua);
    porla::Lua::Types::UwsApp::Register(m_lua);
    porla::Lua::Types::Zip::Register(m_lua);

    porla::Lua::Packages::Sqlite::Register(m_lua);

    const auto porla_lua = fs.open("porla.lua");

    m_lua.script_file("/workspaces/porla/lua/porla.lua");
    m_lua["on_load"]();
}
