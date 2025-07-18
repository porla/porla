#pragma once

#include "method.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsPeersAdd : public Method<TorrentsPeersAddReq, TorrentsPeersAddRes, SSL>
    {
    public:
        explicit TorrentsPeersAdd(porla::Sessions& sessions);

        void Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes, SSL> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
