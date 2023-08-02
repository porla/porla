#pragma once

#include "method.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsTrackersList : public Method<TorrentsTrackersListReq, TorrentsTrackersListRes>
    {
    public:
        explicit TorrentsTrackersList(Sessions& sessions);

    protected:
        void Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
