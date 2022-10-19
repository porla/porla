#pragma once

#include "method.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsList : public Method<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(porla::ISession& session);

        void Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb) override;

    private:
        porla::ISession& m_session;
    };
}
