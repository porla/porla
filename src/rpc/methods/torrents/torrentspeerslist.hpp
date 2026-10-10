#pragma once

#include "../../typedmethod.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPeersList : public TypedMethod<TorrentsPeersListReq, TorrentsPeersListRes>
    {
    public:
        explicit TorrentsPeersList(porla::Sessions& sessions);

        void Execute(const TorrentsPeersListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
