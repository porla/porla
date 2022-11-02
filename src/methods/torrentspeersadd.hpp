#pragma once

#include "method.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsPeersAdd : public Method<TorrentsPeersAddReq, TorrentsPeersAddRes>
    {
    public:
        explicit TorrentsPeersAdd(porla::ISession& session);

        void Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes> cb) override;

    private:
        porla::ISession& m_session;
    };
}
