#pragma once

#include "method.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsTrackersList : public Method<TorrentsTrackersListReq, TorrentsTrackersListRes, SSL>
    {
    public:
        explicit TorrentsTrackersList(Sessions& sessions);

    protected:
        void Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
