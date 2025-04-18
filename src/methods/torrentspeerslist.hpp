#pragma once

#include "method.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsPeersList : public Method<TorrentsPeersListReq, TorrentsPeersListRes, SSL>
    {
    public:
        explicit TorrentsPeersList(porla::Sessions& sessions);

        void Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes, SSL> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
