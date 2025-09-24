#pragma once

#include "method.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPause : public Method<TorrentsPauseReq, TorrentsPauseRes>
    {
    public:
        explicit TorrentsPause(porla::Sessions& session);

    protected:
        void Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
