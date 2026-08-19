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
    DECLARE_TYPE_REGISTRY(LtAnnounceEndpoint)
    DECLARE_TYPE_REGISTRY(LtAnnounceEntry)
    DECLARE_TYPE_REGISTRY(LtAnnounceInfohash)
    DECLARE_TYPE_REGISTRY(LtOpenFileState)
    DECLARE_TYPE_REGISTRY(LtPeerInfo)
    DECLARE_TYPE_REGISTRY(LtTorrentHandle)
    DECLARE_TYPE_REGISTRY(LtTorrentStatus)
}
