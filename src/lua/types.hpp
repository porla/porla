#pragma once

#include <sol/sol.hpp>

#define DECLARE_TYPE_REGISTRY(name) \
    class name                \
    {                         \
    public:                   \
        static void Register(sol::state& lua); \
    };

namespace porla::Lua::Types
{
    DECLARE_TYPE_REGISTRY(Fs)
    DECLARE_TYPE_REGISTRY(Log)
    DECLARE_TYPE_REGISTRY(LtAddTorrentParams)
    DECLARE_TYPE_REGISTRY(LtAlert)
    DECLARE_TYPE_REGISTRY(LtAnnounceEndpoint)
    DECLARE_TYPE_REGISTRY(LtAnnounceEntry)
    DECLARE_TYPE_REGISTRY(LtAnnounceInfohash)
    DECLARE_TYPE_REGISTRY(LtDownloadPriority)
    DECLARE_TYPE_REGISTRY(LtFileStorage)
    DECLARE_TYPE_REGISTRY(LtInfoHash)
    DECLARE_TYPE_REGISTRY(LtPeerInfo)
    DECLARE_TYPE_REGISTRY(LtSession)
    DECLARE_TYPE_REGISTRY(LtSettingsPack)
    DECLARE_TYPE_REGISTRY(LtStorageMode)
    DECLARE_TYPE_REGISTRY(LtTorrentFlags)
    DECLARE_TYPE_REGISTRY(LtTorrentHandle)
    DECLARE_TYPE_REGISTRY(LtTorrentInfo)
    DECLARE_TYPE_REGISTRY(LtTorrentStatus)
    DECLARE_TYPE_REGISTRY(Mmdb)
    DECLARE_TYPE_REGISTRY(Toml)
    DECLARE_TYPE_REGISTRY(UwsApp)
    DECLARE_TYPE_REGISTRY(Zip)
}
