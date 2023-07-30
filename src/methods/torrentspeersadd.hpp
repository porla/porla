#pragma once

#include "method.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPeersAdd : public Method<TorrentsPeersAddReq, TorrentsPeersAddRes>
    {
    public:
        explicit TorrentsPeersAdd(porla::Sessions& sessions);

        void Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
