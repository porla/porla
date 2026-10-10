#pragma once

#include "../../typedmethod.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsTrackersList : public TypedMethod<TorrentsTrackersListReq, TorrentsTrackersListRes>
    {
    public:
        explicit TorrentsTrackersList(porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsTrackersListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
