#pragma once

#include "../../typedmethod.hpp"
#include "torrentstrackersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsTrackersAdd : public TypedMethod<TorrentsTrackersAddReq, TorrentsTrackersAddRes>
    {
    public:
        explicit TorrentsTrackersAdd(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsTrackersAddReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
