#pragma once

#include <memory>
#include <optional>

#include <libtorrent/info_hash.hpp>

#include "../../responsewriter.hpp"

namespace porla
{
    class Session;
    class Sessions;
    struct Torrent;
}

namespace porla::Rpc::Methods::Torrents
{
    struct ResolvedTorrent
    {
        std::shared_ptr<porla::Session> session;
        const porla::Torrent*           torrent;
    };

    std::shared_ptr<porla::Session> ResolveSession(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const ResponseWriterHandle& cb);

    std::optional<ResolvedTorrent> ResolveTorrent(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const lt::info_hash_t& hash,
        const ResponseWriterHandle& cb);
}
