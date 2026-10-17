#pragma once

#include <memory>
#include <optional>

#include <libtorrent/info_hash.hpp>

#include "../../data/models/sessions.hpp"
#include "../responsewriter.hpp"

namespace porla
{
    class Session;
    class Sessions;
    struct Torrent;
}

namespace porla::Rpc::Methods
{
    struct ResolvedSession
    {
        Data::Models::Sessions::Session record;
        std::shared_ptr<porla::Session> session;
    };

    struct ResolvedTorrent
    {
        std::shared_ptr<porla::Session> session;
        const porla::Torrent*           torrent;
    };

    Data::Models::Sessions::Session FindSession(
        porla::Sessions& sessions,
        std::optional<int> session_id);

    ResolvedSession ResolveSession(
        porla::Sessions& sessions,
        std::optional<int> session_id);

    std::optional<ResolvedTorrent> ResolveTorrent(
        porla::Sessions& sessions,
        std::optional<int> session_id,
        const lt::info_hash_t& hash,
        const ResponseWriterHandle& cb);
}
