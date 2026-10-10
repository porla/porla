#pragma once

#include "../../typedmethod.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsList : public TypedMethod<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(porla::Sessions& sessions);

        void Execute(const TorrentsListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
