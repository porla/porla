#pragma once

#include "method.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsMove : public Method<TorrentsMoveReq, TorrentsMoveRes>
    {
    public:
        explicit TorrentsMove(Sessions& sessions);

    protected:
        void Invoke(const TorrentsMoveReq& req, WriteCb<TorrentsMoveRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
