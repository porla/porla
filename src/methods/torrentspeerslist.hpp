#pragma once

#include "method.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsPeersList : public Method<TorrentsPeersListReq, TorrentsPeersListRes>
    {
    public:
        explicit TorrentsPeersList(porla::ISession& session);

        void Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb) override;

    private:
        porla::ISession& m_session;
    };
}
