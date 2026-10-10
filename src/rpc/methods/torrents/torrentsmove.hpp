#pragma once

#include "../../typedmethod.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsMove : public TypedMethod<TorrentsMoveReq, TorrentsMoveRes>
    {
    public:
        explicit TorrentsMove(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsMoveReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
