#pragma once

#include "method.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsMove : public Method<TorrentsMoveReq, TorrentsMoveRes, SSL>
    {
    public:
        explicit TorrentsMove(Sessions& sessions);

    protected:
        void Invoke(const TorrentsMoveReq& req, WriteCb<TorrentsMoveRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
