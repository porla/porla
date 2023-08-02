#pragma once

#include "method.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPeersList : public Method<TorrentsPeersListReq, TorrentsPeersListRes>
    {
    public:
        explicit TorrentsPeersList(porla::Sessions& sessions);

        void Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
