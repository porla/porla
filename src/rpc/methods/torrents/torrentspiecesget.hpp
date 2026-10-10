#pragma once

#include "../../typedmethod.hpp"
#include "torrentspiecesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPiecesGet : public TypedMethod<TorrentsPiecesGetReq, TorrentsPiecesGetRes>
    {
    public:
        explicit TorrentsPiecesGet(porla::Sessions& sessions);

        void Execute(const TorrentsPiecesGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
