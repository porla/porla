#pragma once

#include "../../typedmethod.hpp"
#include "torrentref.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPiecesGet : public TypedMethod<TorrentRef>
    {
    public:
        explicit TorrentsPiecesGet(porla::Sessions& sessions);

        void Execute(const TorrentRef& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
