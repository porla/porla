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
        explicit TorrentsMove(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsMoveReq& req, WriteCb<TorrentsMoveRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
