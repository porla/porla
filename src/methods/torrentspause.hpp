#pragma once

#include "method.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsPause : public Method<TorrentsPauseReq, TorrentsPauseRes, SSL>
    {
    public:
        explicit TorrentsPause(Sessions& session);

    protected:
        void Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
