#include "host.hpp"

#include "packages.hpp"
#include "registry.hpp"
#include "types.hpp"

using porla::Lua::Host;

Host::Host(boost::asio::io_context& io)
    : m_io(io)
{
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

    porla::Lua::Types::LtAddTorrentParams::Register(m_lua);
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

    porla::Lua::Packages::Sqlite::Register(m_lua);

    const auto porla_lua = fs.open("porla.lua");

    m_lua.script_file("/workspaces/porla/lua/porla.lua");
}
