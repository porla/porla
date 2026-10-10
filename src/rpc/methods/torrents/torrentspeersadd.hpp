#pragma once

#include "../../typedmethod.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPeersAdd : public TypedMethod<TorrentsPeersAddReq, TorrentsPeersAddRes>
    {
    public:
        explicit TorrentsPeersAdd(porla::Sessions& sessions);

        void Execute(const TorrentsPeersAddReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
