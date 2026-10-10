#pragma once

#include "../../typedmethod.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPause : public TypedMethod<TorrentsPauseReq, TorrentsPauseRes>
    {
    public:
        explicit TorrentsPause(porla::Sessions& session);

    protected:
        void Execute(const TorrentsPauseReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
