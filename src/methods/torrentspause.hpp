#pragma once

#include "method.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsPause : public Method<TorrentsPauseReq, TorrentsPauseRes>
    {
    public:
        explicit TorrentsPause(ISession& session);

    protected:
        void Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes> cb) override;

    private:
        ISession& m_session;
    };
}
