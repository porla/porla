#pragma once

#include "method.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsMove : public Method<TorrentsMoveReq, TorrentsMoveRes>
    {
    public:
        explicit TorrentsMove(ISession& session);

    protected:
        void Invoke(const TorrentsMoveReq& req, WriteCb<TorrentsMoveRes> cb) override;

    private:
        ISession& m_session;
    };
}
