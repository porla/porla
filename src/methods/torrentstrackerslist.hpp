#pragma once

#include "method.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsTrackersList : public Method<TorrentsTrackersListReq, TorrentsTrackersListRes>
    {
    public:
        explicit TorrentsTrackersList(ISession& session);

    protected:
        void Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb) override;

    private:
        ISession& m_session;
    };
}
